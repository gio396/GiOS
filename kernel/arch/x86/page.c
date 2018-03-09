#include "page.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/io.h>

#include <mem_layout.h>

#include <macros.h>
#include <assert.h>
#include <string.h>

extern const u32 l_ekernel;
u32 *page_directory_entry;
u32 *first_page_table;

//single table directory entry
//    31         11    9  8  7  6  5  4  3  2  1  0
//PDT [ADDR-*20*-|AVL--|G-|S-|0-|A-|D-|W-|U-|R-|P-]
//ADDR: Physical adress of array of page table entries.
//AVL: not used by processor OS can use it to store information.
//G: ignored
//S: Page Size. Of this bit is set page size is equal to 4mb otherwise 4kb
//A: Accessed. If bit is set than page has been read from or written to.(needs to be cleared manually).
//D: Cache Disabled. If bit is set the page will not be cached.
//W: Write-Through. If bit is set Write-Through caching is enabled otherwise not write-back is enabled 
//U: User/Superuser. If is set the page will be user page.
//R: Read-Write. If bit is set page will be Readable and Writable otherwise Readable only.
//P: Present. If bit is set page is present in actual physical memory. 
//   If page is called but not pressent page fault will occur and OS will ned to handle it
//   If present bit is not set os can use rest of 31 bits to store information.

//single Page table entry
//    31         11    9  8  7  6  5  4  3  2  1  0
//PTE [ADDR-*20*-|AVL--|G-|0-|D-|A-|C-|W-|U-|R-|P-]
//ADDR: Physical adress where virtual adress should be mapped to.
//AVL: not used by processor OS can use it to store information.
//G: The global. If bit is set prevents TLB form updating adress in its cache if cr3 is reset.
//   Global enable bit must be set in cr4 register for this to work.
//D: Dirty. If set the page has been written to.
//   CPU will not be unset this flag. must be updated by os.
//A: Accessed. If bit is set than page has been read from or written to.(needs to be cleared manually).
//C: Cache Disabled. If bit is set the page will not be cached.
//W: Write-Through. If bit is set Write-Through caching is enabled otherwise not write-back is enabled 
//U: User/Superuser. If is set the page will be user page.
//R: Read-Write. If bit is set page will be Readable and Writable otherwise Readable only.
//P: Present. If bit is set page is present in actual physical memory. 
//   If page is called but not pressent page fault will occur and OS will ned to handle it
//   If present bit is not set os can use rest of 31 bits to store information.


#define SEG_ADR(x) ((x)) //PDT PTE
#define SEG_AVL(x) ((x) << 0x09) //PDT PTE
#define SEG_IGN(x) ((x) << 0x08) //0   PTE
#define SEG_PGS(x) ((x) << 0x07) //PDT 0
#define SEG_DRT(x) ((x) << 0x06) //0   PTE 
#define SEG_ACC(x) ((x) << 0x05) //PDT PTE
#define SEG_CHD(x) ((x) << 0x04) //PDT PTEmap
#define SEG_WRT(x) ((x) << 0x03) //PDT PTE
#define SEG_USU(x) ((x) << 0x02) //PDT PTE
#define SEG_RDW(x) ((x) << 0x01) //PDT PTE
#define SEG_PRE(x) ((x) << 0x00) //PDT PTE

#define EMPTY_PAGE SEG_ADR(0) | SEG_AVL(0) | SEG_IGN(0) | SEG_PGS(0) |\
                   SEG_DRT(0) | SEG_ACC(0) | SEG_CHD(0) | SEG_WRT(0) |\
                   SEG_USU(0) | SEG_RDW(1) | SEG_PRE(0)

#define SY4MB_PAGE SEG_ADR(0) | SEG_AVL(0) | SEG_IGN(0) | SEG_PGS(1) |\
                   SEG_DRT(0) | SEG_ACC(0) | SEG_CHD(0) | SEG_WRT(0) |\
                   SEG_USU(1) | SEG_RDW(1) | SEG_PRE(1)


#define EMPTY_PRESENT(a)   SEG_ADR(a) | SEG_AVL(0) | SEG_IGN(0) | SEG_PGS(0) |\
                           SEG_DRT(0) | SEG_ACC(0) | SEG_CHD(0) | SEG_WRT(0) |\
                           SEG_USU(0) | SEG_RDW(1) | SEG_PRE(1)

#define APIC_VIRTUAL_BASE  0xFEE00000

#define GET_VIRT_DIRECTORY_OFFSET(p)  ((u32)p >> 22)
#define GET_VIRT_PAGE_TABLE_OFFSET(p) ((((u32)p) << 10) >> 22)

void
init_buddy_system(u32 mem_size);

void 
free_range(void *begin, void *end)
{
  assert1(begin);
  assert1(end);
  assert1(ALIGNED(begin, kb(4)));
  assert1(ALIGNED(begin, kb(4)));

  u8* p = (u8*)(begin);

  for(; p <= (u8*)end; p+=PGSIZE) 
    kfree(p);
}

void
page_init()
{
  u32 addr = ((((u32)(&l_ekernel)) + 4096) & 0xFFFFF000);
  page_directory_entry = (u32*)(addr);
  first_page_table = (u32*)(addr) + 1024;

  printk(&state, "Page directory entries addr: 0x%8X\n", page_directory_entry);
  printk(&state, "First page table addr:       0x%8X\n", first_page_table);

  memset(page_directory_entry, 0, kb(4));

  //empty out all pages
  for (u32 p = 0; p < (i32)(KERNEL_VIRTUAL_BASE >> 22); p++)
  {
    page_directory_entry[p] = EMPTY_PRESENT(VIRT2PHYS(first_page_table) + p * 0x1000);
  }

  //enable apic page directory entry
  page_directory_entry[(APIC_VIRTUAL_BASE) >> 22] = (SEG_ADR(0xFEC00000) | SY4MB_PAGE | SEG_CHD(1));

  //4 mb map for higher half kernel.
  page_directory_entry[KERNEL_VIRTUAL_BASE >> 22] = SY4MB_PAGE;
  page_directory_entry[(KERNEL_VIRTUAL_BASE >> 22) + 1] = (SEG_ADR(0x400000) | SY4MB_PAGE);

  enable_paging((u32)page_directory_entry - KERNEL_VIRTUAL_BASE);

  memset(first_page_table, 0, mb(4));

  for (u32 p = 0; p < (i32)(KERNEL_VIRTUAL_BASE >> 22); p++)
  {
    for(u32 j = 0; j < 1024; j++)
    {
      first_page_table[p * 1024 + j] = EMPTY_PRESENT((p) * 0x400000 + 0x1000 * j);
    }
  }

  first_page_table[0] = EMPTY_PAGE;
  init_buddy_system(mb(64));
}

struct free_page_list
{
  struct free_page_list *next;
};

struct
{
  struct free_page_list *list;
} kmem;

u8*
buddy_alloc(u32 size);

void
buddy_free(void *addr);

void*
kalloc(u32 page_count)
{
  return buddy_alloc(kb(4) * page_count); 
}

void
kfree(void* v)
{
  assert1(v);
  assert1(ALIGNED((u32)v, kb(4)));

  buddy_free(v);
}

void
mmap(void *paddr, u32 size, u8 flags)
{
  UNUSED(flags);
 //TODO(gio): ignoring flags for now...

  // assert1(ALIGNED((size_t)paddr, kb(4)));
  u32 page_count = (size + kb(4) - 1) / (kb(4));
  u32 pgindex = GET_VIRT_DIRECTORY_OFFSET(paddr);

  if (page_directory_entry[pgindex] == 0)
  {
    void*  new_page_table = kalloc(1);
    page_directory_entry[pgindex] = EMPTY_PRESENT((u32)new_page_table);
    memset(new_page_table, 0, kb(4));
  }
  else if (page_directory_entry[pgindex] && (SY4MB_PAGE) == 1)
  {
    //already mapped!
    return;
  }

  u32 *page_table = (u32*)(page_directory_entry[pgindex] & 0xFFFFF800);
  u32 page_table_offset = GET_VIRT_PAGE_TABLE_OFFSET(paddr);

  for (u32 i = 0; i < page_count; i++)
  {
    if (page_table[page_table_offset + page_count] == 0)
    {
      page_table[page_table_offset + i] = EMPTY_PRESENT((u32)paddr + 0x1000 * i);
    }
  }
}

#define DYNAMIC_MEMORY_BASE   mb(16)
#define BUDDY_MIN_REGION_SIZE kb(4)
#define BUDDY_MAX_REGION_SIZE mb(8)
//NOTE(gio): change max depth when chaning regions sizes.
#define BUDDY_MAX_DEPTH       (12)
#define BUDDY_NODES_COUNT     ((1 << BUDDY_MAX_DEPTH) - 1)
#define BYTES_PER_BUDDY_TREE  ((BUDDY_NODES_COUNT + 7) >> 3)

struct buddy_tree
{
  char tree[BYTES_PER_BUDDY_TREE];
};

struct buddy_system
{
  u32 buddy_count;
  u32 mem_size;
  struct buddy_tree array[];
};

static u32
get_buddy_count(u32 mem_size)
{
  u32 buddy_count = (mem_size + BUDDY_MAX_REGION_SIZE - 1) / BUDDY_MAX_REGION_SIZE;
  return buddy_count;
}

static u32
lzcnt(u32 val)
{
  u32 res;
  __lzcnt(res, val);

  return res;
}

static i8
get_tree_node(struct buddy_tree *tree, u32 idx)
{
  u32 byte = idx / 8;
  u32 offset = idx % 8;

  return (tree -> tree[byte] >> offset & 0x1);
}

void
set_tree_node(struct buddy_tree *tree, u32 idx, u32 val)
{
  u32 byte = idx / 8;
  u32 offset = idx % 8;

  if (val)
  {
    tree -> tree[byte] = SET_BIT(tree -> tree[byte], 1 << offset);
  }
  else
  {
    tree -> tree[byte] = CLEAR_BIT(tree -> tree[byte], 1 << offset);
  }
}

static u8*
get_tree_node_addr(u32 node, u32 cur_depth)
{
  u32 size_per_node = BUDDY_MIN_REGION_SIZE << (BUDDY_MAX_DEPTH - cur_depth);
  u32 ofsset_from_first = node - ((1 << (cur_depth - 1)) - 1);
  return (u8*)(size_per_node * ofsset_from_first);
}

#define IS_SET(tree, idx) get_tree_node(tree, idx)
#define SET(tree, idx)    set_tree_node(tree, idx, 1)
#define UNSET(tree, idx)  set_tree_node(tree, idx, 0)
#define LC(n) (((n) * 2) + 1)
#define RC(n) (((n) * 2) + 2)
#define P(n)  (((n) - 1) / 2)
#define B(n)  (-((-(n)) ^ 0x1))

#define TREE_BASE_ADDR(i) (u8*)(BUDDY_MAX_REGION_SIZE * i)
#define TREE_ID_FROM_ADDR(addr) (((size_t)addr - DYNAMIC_MEMORY_BASE) / BUDDY_MAX_REGION_SIZE)
#define GET_NODE_FROM_ADDR(addr) ((size_t)addr / BUDDY_MIN_REGION_SIZE)

i8
buddy_tree_try_alloc_depth(struct buddy_tree *tree, i32 depth, u32 *addr)
{
  struct 
  {
    i32 depth;
    u32 node;
  } stack[2 * BUDDY_MAX_DEPTH];
  u32 shead = 1;
  stack[0].depth = 1;
  stack[0].node = 0;

  #define STACK_POP(n, d) do  \
  {                           \
    shead--;                  \
    (n) = stack[shead].node;  \
    (d) = stack[shead].depth; \
  } while (0)                 \

  #define STACK_PUSH(n, d) do \
  {                           \
    stack[shead].node = (n);  \
    stack[shead].depth = (d); \
    shead++;                  \
  } while (0)                 \

  while (shead > 0)
  {
    u32 node;
    i32 cur_depth;
    STACK_POP(node, cur_depth);

    if (depth == cur_depth)
    {
      if (!IS_SET(tree, node))
      {
        SET(tree, node);
        *addr = (size_t)get_tree_node_addr(node, cur_depth);
        return 1;
      }

      continue;
    }

    u32 lc = LC(node);
    u32 rc = RC(node);

    if (!IS_SET(tree, node))
    {
      SET(tree, node);
      STACK_PUSH(lc, cur_depth + 1);
      continue;
    }

    if (!IS_SET(tree, lc) && !IS_SET(tree, rc))
      continue;

    if (IS_SET(tree, rc))
    {
      STACK_PUSH(lc, cur_depth + 1);
      STACK_PUSH(rc, cur_depth + 1);
    }
    else
    {
      STACK_PUSH(rc, cur_depth + 1);
      STACK_PUSH(lc, cur_depth + 1);
    }
  }

  return 0;
}

u8*
buddy_alloc_depth(u32 depth)
{
  struct buddy_system *bs = (struct buddy_system*)DYNAMIC_MEMORY_BASE;

  for (u32 i = 0; i < bs -> buddy_count; i++)
  {
    u32 addr;
    if (buddy_tree_try_alloc_depth(&bs -> array[i], depth, &addr))
    {
      return TREE_BASE_ADDR(i) + addr;
    }
  }

  assert1(0);
  return NULL;
}

u8*
buddy_alloc(u32 size)
{
  u32 logsize = lzcnt(size - 1) + 1;
  u32 asize = (1 << ((lzcnt(size - 1) + 1)));
  asize = asize < BUDDY_MIN_REGION_SIZE ? BUDDY_MIN_REGION_SIZE : asize;

  i32 depth = BUDDY_MAX_DEPTH - ((i32)logsize - 12);

  assert1(depth > 0);
  u8 *addr = DYNAMIC_MEMORY_BASE + buddy_alloc_depth(depth);

  return addr;
}

void
buddy_tree_unset_merge_nodes(struct buddy_tree *tree, u32 node)
{
  UNSET(tree, node);

  while (node)
  {
    u32 buddy = B(node);

    if (IS_SET(tree, buddy))
    {
      break;
    }

    node = P(node);
    UNSET(tree, node);
  }

  if (node == 0)
  {
    UNSET(tree, node);
  }
}

void
buddy_tree_free_addr(struct buddy_tree *tree, void *addr)
{
  u8 *base_addr = TREE_BASE_ADDR(TREE_ID_FROM_ADDR(addr));
  u8 *relative_addr = (u8*)((u8*)addr - base_addr);
  u32 node = GET_NODE_FROM_ADDR(relative_addr);

  while (node != 0)
  {
    if (IS_SET(tree, node))
    {
      buddy_tree_unset_merge_nodes(tree, node);
      return;
    }

    node = P(node);
  }
}

void
buddy_free(void *addr)
{
  u32 id = TREE_ID_FROM_ADDR(addr);
  struct buddy_system *bs = (struct buddy_system*)DYNAMIC_MEMORY_BASE;
  struct buddy_tree *tree = &bs -> array[id];

  buddy_tree_free_addr(tree, addr);
}

void
init_buddy_system(u32 mem_size)
{
  // u32 *buddy_base = (u32*)DYNAMIC_MEMORY_BASE;
  u32 buddy_count = get_buddy_count(mem_size);
  u32 buddy_required_size = ALIGN(buddy_count * sizeof(struct buddy_tree) + sizeof(struct buddy_system), kb(4));
  memset((u8*)DYNAMIC_MEMORY_BASE, 0, buddy_required_size);

  struct buddy_system *bs = (struct buddy_system*)DYNAMIC_MEMORY_BASE;
  bs -> buddy_count = buddy_count;
  bs -> mem_size = mem_size;

  buddy_alloc(buddy_required_size);
}