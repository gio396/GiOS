#ifndef __APCI_H__
#define __APCI_H__

#include <common.h>

struct RSDP_descriptor
{
  int8 signature[8];
  uint8 checksum;
  int8 OEMID[6];
  uint8 revision;
  uint32 rsdt_addr;
} att_packed;

void
find_RSDP();

void
find_RSDP_descriptor(int8 *signature);


#endif
