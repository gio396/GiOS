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

void*
memcpy(const void* s, void* d, size_t n)
{
  const uint8* sp = (uint8*)s;
  uint8* dp = (uint8*)d;

  for(size_t i = 0; i < n; i++)
  {
    dp[i] = sp[i];
  }

  return d;
}