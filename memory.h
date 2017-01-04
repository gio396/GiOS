#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

void *
memset(void* s, int32 c, size_t n);

int32 
check_a20();                   

int32
enable_pmode();

int32
check_pmode();

int32
itoa(int32 in, char* buffer);

extern void 
isr0();

#endif