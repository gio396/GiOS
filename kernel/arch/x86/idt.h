#ifndef __IDT_H__
#define __IDT_H__

#include "common.h"

#define IDT_SIZE 256

struct idt_ptr
{
  i16 limit;
  u32 base;
} att_packed;

struct idt_entry
{
  i16 base_low;
  i16 sel; // kernel segment

  u8 zero; //0

  u8 flags;
  i16 base_high;
} att_packed;


struct idt_entry idt[IDT_SIZE];
struct idt_ptr   idtp;

void
idt_install();

void
set_irq_gates();

#endif