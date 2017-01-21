#include "page.h"
#include <arch/x86/framebuffer.h>

extern const uint32 l_ekernel;

uint32 *page_directory_entry;
uint32 *first_page_table;

//[avai_size, pointer to the head of ll|]


void
page_init()
{
  uint32 addr = ((((uint32)(&l_ekernel)) + 4096) & 0xFFFFF000);
  page_directory_entry = (uint32*)(addr);
  first_page_table = (uint32*)(addr) + 1024;

  printk(&state, "Page directory entries addr: 0x%8X\n", page_directory_entry);
  printk(&state, "First page table addr:       0x%8X\n", first_page_table);

  //empty out all pages

  for (int p = 0; p < PAGE_DIRECTORY_SIZE; p++)
  {
    page_directory_entry[p] = 0x00000002;
  }

  //init first page table

  for (int t = 0; t < PAGE_TABLE_SIiZE; t++)
  {
    first_page_table[t] = (t * 0x1000) | 3;
  }

  page_directory_entry[0] = ((uint32)first_page_table | 3);

  enable_paging((uint32)page_directory_entry);
}

void*
kalloc()
{
  return 0; 
}