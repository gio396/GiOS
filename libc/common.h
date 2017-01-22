#ifndef __COMMON_H__
#define __COMMON_H__

typedef long long int          int64;
typedef unsigned long long int uint64;

typedef int          int32;
typedef unsigned int uint32;
typedef int          b32;

typedef short          int16;
typedef unsigned short uint16;
typedef short          b16;

typedef char          int8;
typedef unsigned char uint8;
typedef char          b8;

typedef float r32;
typedef double r64;

typedef int32 size_t;

#define internal       static
#define global_persist static
#define local_persist  static

#define global

#define FALSE 0
#define TRUE !FALSE

#define NULL (void*)(0)

#define set_att(att) __attribute__((att))
#define set_att_2(att, val) __attribute__((att(val)))

#define att_packed set_att(packed);
#define att_aligned(val) set_att_2(aligned, val)

#endif
