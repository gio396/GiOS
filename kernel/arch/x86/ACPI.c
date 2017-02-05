#include "ACPI.h"

#include <arch/x86/framebuffer.h>

#include <string.h>

#define SIGNATURE "RSD PTR "
#define MEM_START (int8*)(0x000E0000)
#define MEM_END   (int8*)(0x000FFFFF)

global struct RSDP_descriptor *rsdp_base;

void
find_RSDP()
{
  for(int8* begin = MEM_START; begin < MEM_END; begin+=16)
  {
    if(strncmp(begin, SIGNATURE, 8) == 0)
    {
      rsdp_base = (struct RSDP_descriptor*)begin;
    }
  }

  uint32 sum = 0;
  for(uint32 i = 0; i < sizeof(struct RSDP_descriptor); i++)
  {
    sum += ((uint8*)(rsdp_base))[i];
  }

  printk(&state, "RSDP table base: %08X\n", (uint32)(rsdp_base));
  printk(&state, "Sum: %08X\n", sum);

  if ((sum & 0xFF) == 0)
  {
    printk(&state, "Valid RSDP table\n");
  }
}