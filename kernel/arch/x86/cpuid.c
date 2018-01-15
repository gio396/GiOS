#include "cpuid.h"

void
cpuid(i32 code, u32 *a, u32 *b, u32 *c, u32 *d)
{
  __asm__ __volatile__ ("cpuid" : 
                        "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d):
                        "a"(code):);
}

void
cpuid_string(i32 code, i8 *buffer)
{
  u32 *intbuffer = (u32*)buffer;
  __asm__ __volatile__("cpuid":
                       "=a"(*(intbuffer + 0)), "=b"(*(intbuffer + 1)), 
                       "=c"(*(intbuffer + 3)), "=d"(*(intbuffer + 2)):
                       "a"(code):);
}
