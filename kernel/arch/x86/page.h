#ifndef __PAGE_H__
#define __PAGE_H__

#include <common.h>

#define NPTE   1024 //number of page table entries
#define PGSIZE 4096 //page size

#define PROT_READ (1 << 0)
#define PROT_WRITE (1 << 1)
#define PROT_EXEC (1 << 2)

//TODO(gio): dinamically create page table entries;
//           add ability to get phys adress maped to virtaual adress.
//           add ability to map virtual adress to physical adress.
//           add ability to unmap virtual adress from physical adress.
//               (note if all the mappings for page directory entry 
//                is clear clear page table and set page directory entry to 0x00000002)

void
page_init();

extern void
enable_paging(u32 page_directory_ptr);

extern i32
check_pmode();

void *
get_physaddr(void *virtaddr);

void
mmap(void *paddr, u32 size, u8 flags);

// alocates 4kb phys memory and returns adresss
// 1 to 1 mapping
void*
kalloc(u32 page_count);

void*
kballoc(size_t size);

//frees 4kb of phys memory must be used on pointer returned by kalloc
//1 to 1 mapping
void
kfree(void *page);

void
init_buddy_system(u32 mem_size);

#endif