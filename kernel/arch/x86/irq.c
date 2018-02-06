#include "irq.h"

#include <arch/x86/idt.h>
#include <arch/x86/io.h> // for outb
#include <arch/x86/framebuffer.h>
#include <arch/x86/apic.h>

#define PIC1 0x20
#define PIC2 0xA0

#define PIC1_CMD PIC1
#define PIC1_DAT (PIC1 + 1)
#define PIC2_CMD PIC2
#define PIC2_DAT (PIC2 + 1)

#define ICW1_ICW4       0x01 // ICW4
#define ICW1_SINGLE     0x02 // Single cascade mdoe
#define ICW1_INTERVAL4  0x04 // call adres interval
#define ICW1_LEVEL      0x08 // Level triggered (edge) mode
#define ICW1_INIT       0x10 // Initialization

#define ICW4_8086       0x01 // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       0x02 // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08 // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C // Buffered mode/master
#define ICW4_SFNM       0x10 // Special fully nested (not)

#define EOI             0x20

typedef void(*PROCIRQHandler)(const union biosregs *reg);

void (*irq_handler_pointer)(const union biosregs *iregs) = NULL;

global void *irq_handlers[] = 
{
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};


//can have only one handler;
void 
irq_set_handler(u8 num, void *handler)
{
  irq_handlers[num] = handler;
}

//same as calling irq_set_handler(num, 0);
void
irq_clear_handler(u8 num)
{
  irq_handlers[num] = 0;
}

void
irq_common_handler(const union biosregs *reg)
{
  u32 irq_handler_index = reg->int_no - 32;
  PROCIRQHandler irq_handler = (PROCIRQHandler)irq_handlers[irq_handler_index];

  if (irq_handler)
  {
    irq_handler(reg);
  }

  //TODO(GIO): write to apic EOI register only when APIC is available and enabled.
  //           otherwise set PIC EOI.
  

  irq_eoi(1, reg->int_no);

  return;
}

void
irq_eoi(b8 apic, i32 intno)
{
  if (intno > 40)
  {
    outb(PIC2_CMD, EOI);
  }

  outb(PIC1_CMD, EOI);

  if (apic)
  {
    apic_write_reg(APIC_EOI_REGISTER, 0);
  }

  return;
}


//remap irq 0 - 8  to [offset1,offset1 + 8)
//remap irq 8 - 16 to [offset2,offset2 + 8)
internal void
irq_remap(u32 offset1, u32 offset2)
{
  u8 m1, m2;

  //save masks.
  m1 = inb(PIC1_DAT);
  m2 = inb(PIC2_DAT);

  outb(PIC1_CMD, ICW1_INIT + ICW1_ICW4);
  outb(PIC2_CMD, ICW1_INIT + ICW1_ICW4);

  outb(PIC1_DAT, offset1);
  outb(PIC2_DAT, offset2);

  outb(PIC1_DAT, 0x04); //tell master PIC slave is at irq2.
  outb(PIC2_DAT, 0x02); //tell slave its cascade identity.

  outb(PIC1_DAT, ICW4_8086); 
  outb(PIC2_DAT, ICW4_8086);

  outb(PIC1_DAT, m1); //restore mask.
  outb(PIC2_DAT, m2); //restore mask.
}


void
irq_install(void)
{
  printk(&state, "Remaping irqs to 32 and 40\n");
  //first 32 interupts are reserved for cpu
  irq_remap(32, 40);

  printk(&state, "Setting irq idt entries\n");
  set_irq_gates();

  printk(&state, "Enabling interupts\n");
  enable_interrupts();
}

void
get_interrupt_masks(u8 *mask1, u8 *mask2)
{
  *mask1 = inb(PIC1_DAT);
  *mask2 = inb(PIC2_DAT);
}

void
set_interrupt_masks(u8 mask1, u8 mask2)
{
  outb(PIC1_DAT, mask1);
  outb(PIC2_DAT, mask2);
}

void
disable_interrupts()
{
  __asm__ __volatile__ ("cli");
}

void
enable_interrupts()
{
  __asm__ __volatile__ ("sti");
}

