#ifndef __STRING_H__
#define __STRING_H__

#include <common.h>

u32
strlen(const i8* string);

i8*
itoa(i32 value, i8* str, u32 base);

i8*
uitoa(u32 value, i8* str, u32 base);

i32
atoi(const i8* str);

i8*
to_upper(i8* str);

b32
strncmp(const i8 *str1, const i8 *str2, size_t num);

b32
strcmp(const i8 *a, const i8 *b);

void*
memset(void* s, i32 c, size_t n);

void*
memcpy(const void* s, void *dst, size_t n);


#endif