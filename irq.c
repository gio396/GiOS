#include "irq.h"

#include "idt.h"
#include "io.h" // for outb
#include "framebuffer.h"

typedef void(*PROCIRQHandler)(const union biosregs *reg);

#define def_irq(num) extern void irq##num(void)

def_irq(0);
def_irq(1);
def_irq(2);
def_irq(3);
def_irq(4);
def_irq(5);
def_irq(6);
def_irq(7);
def_irq(8);
def_irq(9);
def_irq(10);
def_irq(11);
def_irq(12);
def_irq(13);
def_irq(14);
def_irq(15);

#undef def_irq

internal void 
sti_enable() { __asm__ __volatile__ ("sti"); }

internal void
def_keyboard_handler(void)
{
  inb(0x60);
}

global void *irq_handlers[] = 
{
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0
};


//can have only one handler;
void 
irq_set_handler(uint8 num, void *handler)
{
  irq_handlers[num] = handler;
}

//same as calling irq_set_handler(num, 0);
void
irq_clear_handler(uint8 num)
{
  irq_handlers[num] = 0;
}

void
irq_common_handler(const union biosregs *reg)
{
  uint32 irq_handler_index = reg->int_no - 32;
  PROCIRQHandler irq_handler = (PROCIRQHandler)irq_handlers[irq_handler_index];

  if(irq_handler)
  {
    irq_handler(reg);
  }

  if (reg->int_no > 40)
  {
    outb(0x21, 0x20); //EOI slave
  }

  outb(0x20, 0x20); //EOI
  return;
}


//remap irq 0 - 15 to 32 - 49 for protected mode
//TODO(gio): add ability to set offset1 for master PIC
//           and offset for slave PIC manually.
internal void
irq_remap(void)
{
  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  outb(0x21, 0x20);
  outb(0xA1, 0x28);

  outb(0x21, 0x04);
  outb(0xA1, 0x02);

  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  outb(0x21, 0x0);
  outb(0xA1, 0x0);
}


void
irq_install(void)
{
  irq_remap();

  idt_set_gate(32, (uint32)irq0,  0x08, 0x8E);
  idt_set_gate(33, (uint32)irq1,  0x08, 0x8E);
  idt_set_gate(34, (uint32)irq2,  0x08, 0x8E);
  idt_set_gate(35, (uint32)irq3,  0x08, 0x8E);
  idt_set_gate(36, (uint32)irq4,  0x08, 0x8E);
  idt_set_gate(37, (uint32)irq5,  0x08, 0x8E);
  idt_set_gate(38, (uint32)irq6,  0x08, 0x8E);
  idt_set_gate(39, (uint32)irq7,  0x08, 0x8E);
  idt_set_gate(40, (uint32)irq8,  0x08, 0x8E);
  idt_set_gate(41, (uint32)irq9,  0x08, 0x8E);
  idt_set_gate(42, (uint32)irq10, 0x08, 0x8E);
  idt_set_gate(43, (uint32)irq11, 0x08, 0x8E);
  idt_set_gate(44, (uint32)irq12, 0x08, 0x8E);
  idt_set_gate(45, (uint32)irq13, 0x08, 0x8E);
  idt_set_gate(46, (uint32)irq14, 0x08, 0x8E);
  idt_set_gate(47, (uint32)irq15, 0x08, 0x8E);

  irq_set_handler(1, def_keyboard_handler);

  sti_enable();
}