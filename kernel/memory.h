#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <common.h>

void*
alligned_alloc(size_t size, size_t allignment);

void*
kmalloc(size_t size);

void*
kzmalloc(size_t size);

#endif
