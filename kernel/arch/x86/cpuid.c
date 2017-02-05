#include "cpuid.h"

void
cpuid(int32 code, uint32 *a, uint32 *b, uint32 *c, uint32 *d)
{
  __asm__ __volatile__ ("cpuid" : 
                        "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d):
                        "a"(code):);
}

void
cpuid_string(int32 code, int8 *buffer)
{
  uint32 *intbuffer = (uint32*)buffer;
  __asm__ __volatile__("cpuid":
                       "=a"(*intbuffer), "=b"(*(intbuffer + 1)), 
                       "=c"(*(intbuffer + 3)), "=d"(*(intbuffer + 2)):
                       "a"(code):);
}
