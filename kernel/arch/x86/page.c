#include "page.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/io.h>

#include <mem_layout.h>

#include <macros.h>
#include <assert.h>
#include <string.h>

extern const uint32 l_ekernel;
uint32 *page_directory_entry;
uint32 *first_page_table;

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

//singlse Page table entry
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
#define SEG_CHD(x) ((x) << 0x04) //PDT PTE
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


void 
free_range(void *begin, void *end)
{
  assert1(begin);
  assert1(end);
  assert1(ALIGNED(begin, kb(4)));
  assert1(ALIGNED(begin, kb(4)));

  uint8* p = (uint8*)(begin);

  for(; p <= (uint8*)end; p+=PGSIZE) 
    kfree(p);
}

void
page_init()
{
  uint32 addr = ((((uint32)(&l_ekernel)) + 4096) & 0xFFFFF000);
  page_directory_entry = (uint32*)(addr);
  first_page_table = (uint32*)(addr) + 1024;

  printk(&state, "Page directory entries addr: 0x%8X\n", page_directory_entry);
  printk(&state, "First page table addr:       0x%8X\n", first_page_table);

  memset(page_directory_entry, 0, kb(4));

  //empty out all pages
  for (uint32 p = 0; p < (int32)(KERNEL_VIRTUAL_BASE >> 22); p++)
  {
    page_directory_entry[p] = EMPTY_PRESENT(VIRT2PHYS(first_page_table) + p * 0x1000);
  }

  //enable apic page directory entry
  page_directory_entry[(APIC_VIRTUAL_BASE) >> 22] = (SEG_ADR(0xFEC00000) | SY4MB_PAGE | SEG_CHD(1));

  //4 mb map for higher half kernel.
  page_directory_entry[KERNEL_VIRTUAL_BASE >> 22] = SY4MB_PAGE;
  page_directory_entry[(KERNEL_VIRTUAL_BASE >> 22) + 1] = (SEG_ADR(0x400000) | SY4MB_PAGE);

  enable_paging((uint32)page_directory_entry - KERNEL_VIRTUAL_BASE);

  memset(first_page_table, 0, mb(4));

  for (uint32 p = 0; p < (int32)(KERNEL_VIRTUAL_BASE >> 22); p++)
  {
    for(uint32 j = 0; j < 1024; j++)
    {
      first_page_table[p * 0x400 + j] = EMPTY_PRESENT((p) * 0x400000 + 0x1000 * j);
    }
  }

  first_page_table[0] = EMPTY_PAGE;

  //make apic page preset.
  first_page_table[((APIC_VIRTUAL_BASE) >> 22) * 1024] = SEG_ADR(APIC_VIRTUAL_BASE) | SEG_AVL(0) | SEG_IGN(0) | SEG_PGS(0) |
                                                                         SEG_DRT(0) | SEG_ACC(0) | SEG_CHD(1) | SEG_WRT(0) |
                                                                         SEG_USU(1) | SEG_RDW(1) | SEG_PRE(1);

  free_range((void*)(mb(8)), (void*)(mb(128)));
}

struct free_page_list
{
  struct free_page_list *next;
};

struct
{
  struct free_page_list *list;
} kmem;

void*
kalloc()
{
  struct free_page_list *head;
  head = kmem.list;

  if(head)
  {
    kmem.list = head->next;
  }

  return (void*)(head); 
}

void
kfree(void* v)
{
  (void)(v);
  assert1(v);
  assert1(ALIGNED((uint32)v, kb(4)));

  struct free_page_list *head;

  head = (struct free_page_list*)(v);
  head->next = kmem.list;
  kmem.list = head;
}