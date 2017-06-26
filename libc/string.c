#include "string.h"

uint32
strlen(const int8* string)
{
  uint32 res = 0;

  while(string[res++] != '\0');

  return(res - 1);
}

int8* 
itoa(int32 value, int8* str, uint32 base)
{
  int8 *rc;
  int8 *ptr;
  int8 *low;

  if ( base < 2 || base > 36 )
  {
      *str = '\0';
      return str;
  }

  rc = ptr = str;
  // Set '-' for negative decimals.
  if ( value < 0 && base == 10 )
  {
      *ptr++ = '-';
  }

  low = ptr;

  do
  {
    *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
    value /= base;
  } while ( value );

  // Terminating the string.
  *ptr-- = '\0';

  // Invert the numbers.
  while ( low < ptr )
  {
      int8 tmp = *low;
      *low++ = *ptr;
      *ptr-- = tmp;
  }
  return rc;
}

int8* 
uitoa(uint32 value, int8* str, uint32 base)
{
  int8 *rc;
  int8 *ptr;
  int8 *low;

  rc = ptr = str;
  low = ptr;

  do
  {
    *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[value % base];
    value /= base;
  } while ( value );
  // Terminating the string.

  *ptr-- = '\0';

  // Invert the numbers.
  while ( low < ptr )
  {
      int8 tmp = *low;
      *low++ = *ptr;
      *ptr-- = tmp;
  }

  return rc;
}


int8*
to_upper(int8 *str)
{
  int8 *rc = str;

  while(*str != '\0')
  {
    if (*str >= 'a' && *str <= 'z')
    {
      *str -= 0x20;
    }

    str++;
  }

  return rc;
}

//TODO(GIO): handle bases (2 - 36)
int32
atoi(const int8* str)
{
  int32 res = 0;

  while(*str >= '0' && *str <= '9')
  {
    res *= 10;
    res += (*str - '0');
    ++str;
  }

  return res;
}

b32
strncmp(const int8 *str1, const int8 *str2, size_t num)
{
  int8 *a = (int8*)str1;
  int8 *b = (int8*)str2;
  
  while((*a == *b) && --num)
  {
    a++;
    b++;
  }

  return *a - *b;
}

void*
memset(void *s, int32 c, size_t n)
{
    uint8* p = (uint8*)s;

    while(n--)
    {
      p[n] = c;
    }

    return s;
}

void*
memcpy(const void* s, void* d, size_t n)
{
  const uint8* sp = (uint8*)s;
  uint8* dp = (uint8*)d;

  while (n--)
  {
    dp[n] = sp[n];
  }

  return d;
}
