#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <common.h>

void*
alligned_alloc(size_t size, size_t allignment);

void*
kmalloc(size_t size);

void*
kzmalloc(size_t size);

void
kmfree(void *addr);

u64
tbeqw(u64 val);

u32
tbedw(u32 val);

u16
tbew(u16 val);

#endif
