#include "page.h"

void
page_init()
{
  //set cr0 here

  //empty out all pages

  for (int p = 0; p < PAGE_DIRECTORY_SIZE; p++)
  {
    page_directory_entry[p] = 0x00000002;
  }

  //init first page table

  for (int t = 0; t < PAGE_TABLE_SIZE; t++)
  {
    first_page_table[t] = (t * 0x1000) | 3;
  }

  page_directory_entry[0] = ((uint32)first_page_table | 3);

  enable_paging((uint32)page_directory_entry);
}