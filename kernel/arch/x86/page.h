#ifndef __PAGE_H__
#define __PAGE_H__

#include <common.h>

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

//TODO(gio): Wrote page frame allocator

//TODO(gio): dinamically create page table entries;
//           add ability to get phys adress maped to virtaual adress.
//           add ability to map virtual adress to physical adress.
//           add ability to unmap virtual adress from physical adress.
//               (note if all the mappings for page directory entry 
//                is clear clear page table and set page directory entry to 0x00000002)

//TODO(gio): implement higher half kernel for userspace 
//           (http://wiki.osdev.org/Higher_Half_Kernel)


//TODO(gio): move this to oage frame allocator
uint32 page_directory_entry[PAGE_DIRECTORY_SIZE] att_aligned(4096);
uint32 first_page_table[PAGE_TABLE_SIZE] att_aligned(4096);
uint32 last_page_table[1024] att_aligned(4096);

void
page_init();

extern void
enable_paging(uint32 page_directory_ptr);

void *
get_physaddr(void *virtaddr);

void
map_page(void* physaddr, void* virtaddr, uint32 flags);

#endif