#include "memory.h"

#include <macros.h>
#include <list.h>
#include <string.h>

#include <arch/x86/page.h>
#include <arch/x86/framebuffer.h>

#define MEM_LIST_HEADER_SIZE 1
#define SLABS_PER_BLOCK      (512 - MEM_LIST_HEADER_SIZE)
#define SLAB_SIZE            8
#define BITMAP_SIZE          (SLABS_PER_BLOCK + 7) / 8

#define SLAB_NO_MEM          (u32)(-1)

struct slist_root free_list;

struct mem_list_node
{
  struct slist_node self;
  u32 free_slabs;

  u8 bitmap[BITMAP_SIZE];
  u8 slabs[SLABS_PER_BLOCK][SLAB_SIZE];
};

struct alloc_header
{
  size_t count;
  size_t magic;
};

struct mem_list_node*
create_mem_list_node()
{
  struct mem_list_node *node = (struct mem_list_node*)kalloc(1);

  slist_insert_head(&free_list, &node -> self);
  node -> free_slabs = SLABS_PER_BLOCK;
  memset(node -> bitmap, 0, SLABS_PER_BLOCK);

  return node;
}

force_inline i8
bitmap_is_set(struct mem_list_node *node, u32 slab)
{
  u32 idx = slab / 8;
  u32 bit = slab % 8;

  return IS_BIT_SET(node -> bitmap[idx], (1 << bit));
}

force_inline void
bitmap_set(struct mem_list_node *node, u32 slab)
{
  u32 idx = slab / 8;
  u32 bit = slab % 8;

  node -> bitmap[idx] = SET_BIT(node -> bitmap[idx], (1 << bit));
}

force_inline void
mem_list_mark_nodes(struct mem_list_node *node, u32 begin, u32 end)
{
  for (u32 i = begin; i < end; i++)
  {
    bitmap_set(node, i);
  }
}


local u32
find_cont_slabs_node(struct mem_list_node *node, u32 slab_count)
{
  u32 slab_base = 0;
  u32 slab_len  = 0;

  for (u32 i = 0; i < SLABS_PER_BLOCK; i++)
  {
    if (bitmap_is_set(node, i))
    {
      slab_len = 0;
      slab_base = i + 1;
      continue;
    }

    slab_len++;

    if (slab_len == slab_count)
    {
      mem_list_mark_nodes(node, slab_base, slab_base + slab_len);
      return slab_base;
    }
  }

  return SLAB_NO_MEM;
}

force_inline size_t
addr_from_slab_id(struct mem_list_node *node, u32 slab_id)
{
  return (size_t)&node -> slabs[slab_id];
}

local size_t
find_cont_slabs(u32 slab_count)
{
  struct slist_node *head = free_list.slist_node;
  struct slist_node *it;
  FOR_EACH_LIST(it, head)
  {
    struct mem_list_node *node = CONTAINER_OF(it, struct mem_list_node, self);

    if (node -> free_slabs < slab_count)
      continue;

    u32 slab = find_cont_slabs_node(node, slab_count);
    if (slab == SLAB_NO_MEM)
      continue;

    return addr_from_slab_id(node, slab);
  }

  return 0;
}

local size_t
find_create_cont_slabs(u32 slab_count)
{
  size_t addr = find_cont_slabs(slab_count);

  if (addr > 0)
    return addr;

  struct mem_list_node *node = create_mem_list_node();
  node -> free_slabs -= slab_count;
  mem_list_mark_nodes(node, 0, slab_count);

  return addr_from_slab_id(node, 0);
}

local void*
allocate_slabs(size_t size)
{
  u32 slab_count = (size + SLAB_SIZE - 1) / SLAB_SIZE + MEM_LIST_HEADER_SIZE;
  size_t base_addr = find_create_cont_slabs(slab_count);

  struct alloc_header *header = (struct alloc_header*)base_addr;
  header -> count = slab_count;
  header -> magic = base_addr ^ size;

  return (void*)(base_addr + SLAB_SIZE * MEM_LIST_HEADER_SIZE);
}

local void*
internal_allocate(size_t size)
{
  i8 *addr;
  if (size > kb(2))
  {
    addr =  kballoc(size);
  }
  else
  {
    addr = allocate_slabs(size);
  }

  return addr;
}

void*
kmalloc(size_t size)
{
  return internal_allocate(size);
}

void*
kzmalloc(size_t size)
{
  void *ptr = internal_allocate(size);
  memset(ptr, 0, size);

  return ptr;
}

u64
tbeqw(u64 val)
{

  return 0;
}

u32
tbedw(u32 val)
{
  return (((val>>24) & 0x000000ff) | ((val>>8) & 0x0000ff00) | ((val<<8) & 0x00ff0000) | ((val<<24) & 0xff000000));
}

u16
tbew(u16 val)
{
  return ((val >> 8) & 0x00ff) | ((val << 8) | 0xff00);
}
