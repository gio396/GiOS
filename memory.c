#include "memory.h"

void*
memset(void *s, int32 c, size_t n)
{
    uint8* p = (uint8*)s;
    for (size_t i = 0; i != n; ++i) 
    {
        p[i] = c;
    }

    return s;
}