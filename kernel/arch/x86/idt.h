#ifndef __IDT_H__
#define __IDT_H__

#include "common.h"
#include <arch/x86/register.h>

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

u32
get_next_irq();

void
subscribe_irq(u32 irq, void *handler, void *data);

void
idt_call_irq(u32 irq, union biosregs *iregs); 

#endif