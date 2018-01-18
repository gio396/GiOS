#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <common.h>

void*
alligned_alloc(size_t size, size_t allignment);

void*
malloc(size_t size);

#endif
