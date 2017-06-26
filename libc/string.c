#include "string.h"

uint32
strlen(const int8* string)
{
  uint32 res = 0;

  while(string[res++] != '\0');

  return(res - 1);
}

int8* 
itoa(int32 value, int8 *str, uint32 base)
{
  int8 *ptr;

  if ( base < 2 || base > 36 )
  {
      *str = '\0';
      return str;
  }

  ptr = str;
  // Set '-' for negative decimals.
  if ( value < 0 && base == 10 )
  {
      value = -value;
      *ptr++ = '-';
  }

  return uitoa(value, ptr, base);;
}

int8* 
uitoa(uint32 value, int8 *str, uint32 base)
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

  *ptr-- = '\0';

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

internal int32
get_value_2_(const int8 *str)
{
  int32 res = 0;
  int8* c = (int8*)(str);

  while (*c == '0' || *c == '1')
  {
    res *= 2;
    res += *c - '0';
    ++c;
  }

  return res;
}

internal int32 
get_value_10_(const int8 *str)
{
  int32 res = 0;
  int8* c = (int8*)(str);

  while(*c >= '0' && *c <= '9')
  {
    res *= 10;
    res += (*c - '0');
    ++c;
  }

  if (*c != '\0')
    return 0;

  return res;
}

internal int32
get_value_16_(const int8 *str)
{
  int32 res = 0;
  int8* c = (int8*)(str);

  while (*c)
  {
    int32 inc;

    if (*c >= '0' && *c <= '9')
    {
      inc = *c - '0';
    }
    else if (*c >= 'a' && *c <= 'f')
    {
      inc = *c - 'a' + 10;
    }
    else if (*c >= 'A' && *c <= 'F')
    {
      inc = *c - 'A' + 10;
    }
    else
    {
      res = 0 / res;
      break;
    }

    res *= 16;
    res += inc;
    ++c;
  }

  return res;
}

internal int32
get_base_(const int8* str)
{
  int8 *c = (int8*)(str);
  int32 res;

  if (*c == '0')
  {
    ++c;

    if (*c == 'b' || *c == 'B')
    {
      res = 2;
    }
    else if (*c == 'x' || *c == 'X')
    {
      res = 16;
    }
    else
    {
      res = 8;
    }
  }
  else
  {
    if (*c == '-')
    {
      res = -10;
    }
    else
    {
      res = 10;
    }
  }

  return res;
}

int32
atoi(const int8 *str)
{
  int32 res = 0;
  int8 *c = (int8*)str;
  b8 neg = 0;

  while((*c == ' ' || *c == '\t') && c++);

  int32 base = get_base_(c);

  if (base == -10)
  {
    c++;
    base = 10;
    neg = 1;
  }

  switch (base)
  {
    case 2:
    {
      c+=2;
      res = get_value_2_(c);
      break;
    }
    case 10:
    {
      res = get_value_10_(c);
      break;
    }
    case 16:
    {
      c+=2;
      res = get_value_16_(c);
      break;
    }
    default:
    {
      res = get_value_10_(c);
      break;
    }
  }

  if (neg)
  {
    res = -res;
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
memcpy(const void *s, void *d, size_t n)
{
  const uint8* sp = (uint8*)s;
  uint8* dp = (uint8*)d;

  while (n--)
  {
    dp[n] = sp[n];
  }

  return d;
}
