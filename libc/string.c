#include "string.h"

u32
strlen(const i8* string)
{
  u32 res = 0;

  while(string[res++] != '\0');

  return(res - 1);
}

i8* 
itoa(i32 value, i8 *str, u32 base)
{
  i8 *ptr;

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

i8* 
uitoa(u32 value, i8 *str, u32 base)
{
  i8 *rc;
  i8 *ptr;
  i8 *low;

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
      i8 tmp = *low;
      *low++ = *ptr;
      *ptr-- = tmp;
  }

  return rc;
}


i8*
to_upper(i8 *str)
{
  i8 *rc = str;

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

internal i32
get_value_2_(const i8 *str)
{
  i32 res = 0;
  i8* c = (i8*)(str);

  while (*c == '0' || *c == '1')
  {
    res *= 2;
    res += *c - '0';
    ++c;
  }

  return res;
}

internal i32 
get_value_10_(const i8 *str)
{
  i32 res = 0;
  i8* c = (i8*)(str);

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

internal i32
get_value_16_(const i8 *str)
{
  i32 res = 0;
  i8* c = (i8*)(str);

  while (*c)
  {
    i32 inc;

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

internal i32
get_base_(const i8* str)
{
  i8 *c = (i8*)(str);
  i32 res;

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

i32
atoi(const i8 *str)
{
  i32 res = 0;
  i8 *c = (i8*)str;
  b8 neg = 0;

  while((*c == ' ' || *c == '\t') && c++);

  i32 base = get_base_(c);

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
strncmp(const i8 *str1, const i8 *str2, size_t num)
{
  i8 *a = (i8*)str1;
  i8 *b = (i8*)str2;
  
  while((*a == *b) && --num)
  {
    a++;
    b++;
  }

  return *a - *b;
}

void*
memset(void *s, i32 c, size_t n)
{
  u8* p = (u8*)s;

  while(n--)
  {
    p[n] = c;
  }

  return s;
}

void*
memcpy(const void *s, void *d, size_t n)
{
  const u8* sp = (u8*)s;
  u8* dp = (u8*)d;

  while (n--)
  {
    dp[n] = sp[n];
  }

  return d;
}

b32
strcmp(const i8 *a, const i8 *b)
{
  i8 *it_a = (i8*)(a);
  i8 *it_b = (i8*)(b);

  while ((*it_a != '\0' || it_b != '\0') && *it_a == *it_b) {it_a++; it_b++};

  return *it_b - *it_a;
}