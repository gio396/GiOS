#ifndef __IDT_H__
#define __IDT_H__

#include "common.h"

#define IDT_SIZE 256

struct idt_ptr
{
  uint16 limit;
  uint32 base;
} att_packed;

struct idt_entry
{
  uint16 base_low;
  uint16 sel; // kernel segment

  uint8 zero; //0

  uint8 flags;
  uint16 base_high;
} att_packed;


struct idt_entry idt[IDT_SIZE];
struct idt_ptr   idtp;

void
idt_install();

void
idt_set_gate(int32 num, uint32 base, uint16 sel, uint8 flags);

#endif