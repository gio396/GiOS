#ifndef __TYPES_H__
#define __TYPES_H__

typedef long long int          i64;
typedef unsigned long long int u64;

typedef int          i32;
typedef unsigned int u32;
typedef int          b32;

typedef short          i16;
typedef unsigned short u16;
typedef short          b16;

typedef char          i8;
typedef unsigned char u8;
typedef char          b8;

typedef float  r32;
typedef double r64;

typedef float  f32;
typedef double f64;

#ifdef __i386__    
typedef i32 size_t;
#else //__x86_64__
typedef i64 size_t;
#endif

#define internal       static
#define global_persist static
#define local_persist  static

#define global 

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array[0])))
#define ARRAY_SIZE(array) (sizeof(array))

#define force_inline __attribute__((always_inline)) inline
#define local  __attribute__ ((visibility ("hidden")))

#define SET_BIT(val, bit)    ((val) |  (bit))
#define CLEAR_BIT(val, bit)  ((val) & ~(bit))
#define TOGGLE_BIT(val, bit) ((val) ^  (bit))
#define IS_BIT_SET(val, bit) ((val) &  (bit))

#define ZERO_STRUCT(struct, type) memset((struct), 0, sizeof(type))

#define USED __attribute__((used))
#define UNUSED(x) (void)(x)

#define NULL (void*)(0)

#define min(lhs, rhs) ((lhs) < (rhs) ? (lhs) : (rhs))
#define max(lhs, rhs) ((lhs) > (rhs) ? (lhs) : (rhs))

#ifdef __GIOS_DEBUG__
#define LOG(...) printk(&state, __VA_ARGS__)
#define LOGV(f, v) printk(&state, #v " " f "\n", v)
#else
#define LOG(...)
#endif

#define ABORT()  hlt()


#define set_att(att) __attribute__((att))
#define set_att_2(att, val) __attribute__((att(val)))

#define att_packed set_att(packed);
#define att_aligned(val) set_att_2(aligned, val)

#define __lzcnt(dst, src) __asm__ __volatile__ ("lzcnt %1, %0\n":"=r"(dst) :"rm"(src))

#endif
