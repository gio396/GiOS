#ifndef __GDT_H__
#define __GDT_H__

#include "common.h"

struct gdt_ptr 
{
  uint16 limit;
  uint32 base;
} att_packed;

struct gdt_entry
{
  uint16 limit_low;
  uint16 base_low;
  
  uint8 base_middle;
  uint8 access;
  uint8 granularity;
  uint8 base_high;
} att_packed;

struct gdt_entry gdt[3];
struct gdt_ptr gp;

extern void
gdt_flush(int32 gp);

void 
gdt_install();

#endif