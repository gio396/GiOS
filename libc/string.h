#ifndef __STRING_H__
#define __STRING_H__

#include <common.h>

uint32
strlen(const int8* string);

int8*
itoa(int32 value, int8* str, uint32 base);

int8*
uitoa(uint32 value, int8* str, uint32 base);

int32
atoi(const int8* str);

int8*
to_upper(int8* str);

b32
strncmp(const int8 *str1, const int8 *str2, size_t num);

void*
memset(void* s, int32 c, size_t n);

void*
memcpy(const void* s, void *dst, size_t n);


#endif