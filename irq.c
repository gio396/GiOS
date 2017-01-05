#include "irq.h"

#include "idt.h"
#include "io.h" // for outb
#include "framebuffer.h"

typedef void(*PROCIRQHandler)(const union biosregs *reg);

internal void 
sti_enable() { __asm__ __volatile__ ("sti"); }

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
//TODO(gio): Add ability to set offset1 for master PIC
//           and offset for slave PIC manually.
//         : Get rid of magic numbers.
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

  sti_enable();
}