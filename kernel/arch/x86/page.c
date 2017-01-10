#include "page.h"

void
page_init()
{
  //set cr0 here

  //empty out all pages

  for (int32 p = 0; p < PAGE_DIRECTORY_SIZE; p++)
  {
    page_directory_entry[p] = 0x00000002;
  }

  //init first page table

  for (int32 t = 0; t < PAGE_TABLE_SIZE; t++)
  {
    first_page_table[t] = (t * 0x1000) | 3;
  }

  page_directory_entry[0] = ((uint32)first_page_table | 3);

  // map last PDE to itself
  for (int32 t = 0; t < PAGE_TABLE_SIZE; t++)
  {
    last_page_table[t] = ((uint32)page_directory_entry + (t * 0x1000)) | 3;
  }

  page_directory_entry[PAGE_DIRECTORY_SIZE - 1] = (uint32)last_page_table | 3;

  enable_paging((uint32)page_directory_entry);
}

// TODO(GIO): fix this
void*
get_physaddr(void *virtaddr)
{
  uint32 pdindex = (uint32)virtaddr >> 22;
  uint32 ptindex = (uint32)virtaddr >> 12 & 0x03FF;
 
  // uint32 *pd = (uint32 *)0xFFFFF000;

  //check whether the PD entry is present.
 
  uint32 *pt = ((uint32 *)0xFFC00000) + (0x400 * pdindex);

  //check whether the PT entry is present.
 
  return (void *)((pt[ptindex] & ~0xFFF) + ((uint32)virtaddr & 0xFFF));
}

internal inline void 
__native_flush_tlb_single(unsigned long addr)
{
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}


// TODO(GIO): fix this
void 
map_page(void *physaddr, void *virtaddr, uint32 flags)
{
  // Make sure that both addresses are page-aligned.
 
  uint32 pdindex = (uint32)virtaddr >> 22;
  uint32 ptindex = (uint32)virtaddr >> 12 & 0x03FF;
 
  // uint32 *pd = (uint32 *)0xFFFFF000;
  // Here you need to check whether the PD entry is present.
  // When it is not present, you need to create a new empty PT and
  // adjust the PDE accordingly.
 
  uint32 *pt = ((uint32 *)0xFFC00000) + (0x400 * pdindex);
  // Here you need to check whether the PT entry is present.
  // When it is, then there is already a mapping present. What do you do now?
 
  pt[ptindex] = ((uint32)physaddr) | (flags & 0xFFF) | 0x01; // Present
 
  // Now you need to flush the entry in the TLB
  // or you might not notice the change.
  __native_flush_tlb_single(pt[ptindex]);
}