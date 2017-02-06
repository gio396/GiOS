#include "acpi.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/io.h>

#include <string.h>
#include <memory.h>

#define SIGNATURE "RSD PTR "
#define MEM_START (int8*)(0x000E0000)
#define MEM_END   (int8*)(0x000FFFFF)

global struct rsdp_descriptor *rsdp_base;
global struct acpi_sdt_header *sdt_table_header;

void
find_rsdp()
{
  uint32 sdt_table_length;

  for(int8* begin = MEM_START; begin < MEM_END; begin+=16)
  {
    if(strncmp(begin, SIGNATURE, 8) == 0)
    {
      struct rsdp_descriptor *current = (struct rsdp_descriptor*)begin;
      uint32 sum = 0;

      for(uint32 i = 0; i < sizeof(struct rsdp_descriptor); i++)
      {
        sum += ((uint8*)(current))[i];
      }

      if((sum & 0xFF) == 0)
      {
        printk(&state, "Found valid RSDP table at adress: %08X\n", (uint32)current);
        rsdp_base = current;
        break;
      }
    }
  }

  printk(&state, "OEM id %c%c%c%c%c%c\n",
        rsdp_base->oem_id[0], rsdp_base->oem_id[1], rsdp_base->oem_id[2],
        rsdp_base->oem_id[3], rsdp_base->oem_id[4], rsdp_base->oem_id[5]);

  sdt_table_header = (struct acpi_sdt_header*)(rsdp_base->rsdt_addr);
  sdt_table_length = sdt_table_header->length;

  //check if it is valid table
  uint8 *sdt_table_bytes = (uint8*)(sdt_table_header);
  uint32 sum = 0;
  for(uint32 i = 0; i < sdt_table_length; i++)
  {
    sum += sdt_table_bytes[i]; 
  }

  if ((sum & 0xFF) != 0)
  {
    printk(&state, "SDT is INVALID\n");
    halt();
  }
}

void *
find_rstd_descriptor(int8* signature)
{
  struct rsdt *rsdt= (struct rsdt*)(sdt_table_header);
  int32 entries = (rsdt->header.length - sizeof(struct acpi_sdt_header)) / 4;

  for (int32 i = 0; i < entries; i++)
  {
    struct acpi_sdt_header *h = (struct acpi_sdt_header *)rsdt->pointer_to_other_sdt[i];
    if (strncmp(h->signature, signature, 4) == 0)
    {
      return (void*)h;
    }
  }

  return NULL;
}
