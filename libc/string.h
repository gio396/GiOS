#ifndef __STRING_H__
#define __STRING_H__

#include <common.h>

uint32
strlen(const int8* string);

int8*
itoa(int32 value, int8* str, uint32 base);

int32
atoi(const int8* str);

int8*
to_upper(int8* str);

#endif