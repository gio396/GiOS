#ifndef __PAGE_H__
#define __PAGE_H__

#include <common.h>

#define NPTE   1024 //number of page table entries
#define PGSIZE 4096 //page size

//TODO(gio): Wrote page frame allocator

//TODO(gio): dinamically create page table entries;
//           add ability to get phys adress maped to virtaual adress.
//           add ability to map virtual adress to physical adress.
//           add ability to unmap virtual adress from physical adress.
//               (note if all the mappings for page directory entry 
//                is clear clear page table and set page directory entry to 0x00000002)

//TODO(gio): implement higher half kernel for userspace 
//           (http://wiki.osdev.org/Higher_Half_Kernel)

void
page_init();

extern void
enable_paging(uint32 page_directory_ptr);

extern int32
check_pmode();

void *
get_physaddr(void *virtaddr);

void
map_page(void* physaddr, void* virtaddr, uint32 flags);

// alocates 4kb phys memory and returns adresss
// 1 to 1 mapping
void*
kalloc();


//frees 4kb of phys memory must be used on pointer returned by kalloc
//1 to 1 mapping
void
kfree(void *page);

#endif