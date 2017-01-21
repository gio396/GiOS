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

//single table directory entry
//    32         11    9  8  7  6  5  4  3  2  1  0
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
//    32         11    9  8  7  6  5  4  3  2  1  0
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


#define SEG_ADR(x) ((x) << 0x0C) //PDT PTE
#define SEG_AVL(x) ((x) << 0x0A) //PDT PTE
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
    page_directory_entry[p] = EMPTY_PAGE;
  }

  //4 mb map for higher half kernel.
  page_directory_entry[KERNEL_VIRTUAL_BASE >> 22] = SY4MB_PAGE;

  enable_paging((uint32)page_directory_entry - KERNEL_VIRTUAL_BASE);
}

void*
kalloc()
{
  return NULL; 
}