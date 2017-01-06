#ifndef __GDT_H__
#define __GDT_H__

#include <common.h>

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
  uint8 access; // [P-|DPL--|DT-|TYPE----|
  uint8 granularity; // [G-|DB-|0-|A-|LIMIT----];
  uint8 base_high;
} att_packed;

struct gdt_entry gdt[5];
struct gdt_ptr gp;

void 
gdt_install();

#endif