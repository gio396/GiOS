#include "string.h"

uint32
strlen(const uint8* string)
{
  uint32 res = 0;

  while(string[res++] != '\0');

  return(res - 1);
}

uint32
itoa(int32 integer, int8* string)
{
  int32 val = integer;
  int32 it = 0;
  int32 len = 1;
  int32 minus = 0;

  while((val /= 16) != 0) 
    len++;

  if (integer < 0)
  {
    string[0] = '-';
    minus = 1;
    integer *= -1;
  }

  for (int32 i = 1; i <= len; i++)
  {
    int32 rem = integer % 16;
    integer = integer / 16;

    if (rem > 9)
    {
      string[len - i + minus] = 'A' + (rem - 10);
    }
    else
    {
      string[len - i + minus] = '0' + rem;
    }
  }

  string[len + minus] = '\0';

  return it;
}