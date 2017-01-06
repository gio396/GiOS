#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

void *
memset(void* s, int32 c, size_t n);

void*
memcpy(const void* s, void *dst, size_t n);

#endif