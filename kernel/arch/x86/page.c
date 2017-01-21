#include "page.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/io.h>

#include <macros.h>


#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024
#define KERNEL_VIRTUAL_BASE 0xC0000000

extern const uint32 l_ekernel;
uint32 *page_directory_entry;
uint32 *first_page_table;

void
page_init()
{
  uint32 addr = ((((uint32)(&l_ekernel)) + 4096) & 0xFFFFF000);
  page_directory_entry = (uint32*)(addr);
  first_page_table = (uint32*)(addr) + 1024;

  printk(&state, "Page directory entries addr: 0x%8X\n", page_directory_entry);
  printk(&state, "First page table addr:       0x%8X\n", first_page_table);

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

  page_directory_entry[768] = ((uint32)first_page_table - KERNEL_VIRTUAL_BASE) | 3;

  enable_paging((uint32)page_directory_entry - KERNEL_VIRTUAL_BASE);
}

void*
kalloc()
{
  return NULL; 
}