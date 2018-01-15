#ifndef __GDT_H__
#define __GDT_H__

#include <common.h>

struct gdt_ptr 
{
  i16 limit;
  u32 base;
} att_packed;

struct gdt_entry
{
  i16 limit_low;
  i16 base_low;
  
  u8 base_middle;
  u8 access; // [P-|DPL--|DT-|TYPE----|
  u8 granularity; // [G-|DB-|0-|A-|LIMIT----];
  u8 base_high;
} att_packed;

struct gdt_entry gdt[6];
struct gdt_ptr gp;

void 
gdt_install();

#endif