#include "idt.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/register.h>
#include <arch/x86/io.h>

#include <string.h>

typedef void(*PROCIRQHandler)(const union biosregs *reg, void *data);
struct irq_handler
{
  void *data;
  PROCIRQHandler callback;
};

struct irq_handler irq_handlers[IDT_SIZE];

#define def_isr(num) extern void isr##num(void)

def_isr(0);
def_isr(1);
def_isr(2);
def_isr(3);
def_isr(4);
def_isr(5);
def_isr(6);
def_isr(7);
def_isr(8);
def_isr(9);
def_isr(10);
def_isr(11);
def_isr(12);
def_isr(13);
def_isr(14);
def_isr(15);
def_isr(16);
def_isr(17);
def_isr(18);
def_isr(19);
def_isr(20);
def_isr(21);
def_isr(22);
def_isr(23);
def_isr(24);
def_isr(25);
def_isr(26);
def_isr(27);
def_isr(28);
def_isr(29);
def_isr(30);
def_isr(31);

#undef def_isr

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

extern void irqTime(void);
extern void irqHigh(void);

#define SEG_PRES(x) ((x) << 0x07) // 1 present 0 not present
#define SEG_PRIV(x) ((x) << 0x06) // privilage ring (0 - 3)
#define SEG_SGAT(x) ((x) << 0x04) // storage gate

#define SEG_TASK_GATE    0x05
#define SEG_16_INTR_GATE 0x06
#define SEG_16_TRAP_GATE 0x07
#define SEG_32_INTR_GATE 0x0E
#define SEG_32_TRAP_GATE 0x0F


#define IDT_INTR_PL0 (u8)(SEG_PRES(1) | SEG_PRIV(0) | SEG_SGAT(0) | \
                             SEG_32_INTR_GATE)

//      8  7      5  4        0
//flags [p-|DPL---|S-|TYPE----]
void 
idt_set_gate(i32 num, u32 base, i16 sel, u8 flags)
{
  printk(&state, "Setting idt gate: %02d, Base 0x%08X, Segment 0x%02X, Flags %08b\n",
         num, base, sel, flags);

  idt[num].base_low =  (base & 0xFFFF);
  idt[num].base_high = ((base >> 16) & 0xFFFF);

  idt[num].sel =   sel;
  idt[num].zero =  0;
  idt[num].flags = flags;
}

void 
idt_install()
{
  printk(&state, "Creating IDT table\n");

  idtp.limit = (sizeof (struct idt_entry) * IDT_SIZE) - 1;
  idtp.base = (size_t)&idt;

  printk(&state, "IDTP limit 0x%4X, base 0x%8X\n", 
         idtp.limit, idtp.base);

  memset (&idt, 0, sizeof (struct idt_entry) * 256);

  idt_set_gate(0,  (size_t)isr0,  0x08, IDT_INTR_PL0);
  idt_set_gate(1,  (size_t)isr1,  0x08, IDT_INTR_PL0);
  idt_set_gate(2,  (size_t)isr2,  0x08, IDT_INTR_PL0);
  idt_set_gate(3,  (size_t)isr3,  0x08, IDT_INTR_PL0);
  idt_set_gate(4,  (size_t)isr4,  0x08, IDT_INTR_PL0);
  idt_set_gate(5,  (size_t)isr5,  0x08, IDT_INTR_PL0);
  idt_set_gate(6,  (size_t)isr6,  0x08, IDT_INTR_PL0);
  idt_set_gate(7,  (size_t)isr7,  0x08, IDT_INTR_PL0);
  idt_set_gate(8,  (size_t)isr8,  0x08, IDT_INTR_PL0);
  idt_set_gate(9,  (size_t)isr9,  0x08, IDT_INTR_PL0);
  idt_set_gate(10, (size_t)isr10, 0x08, IDT_INTR_PL0);
  idt_set_gate(11, (size_t)isr11, 0x08, IDT_INTR_PL0);
  idt_set_gate(12, (size_t)isr12, 0x08, IDT_INTR_PL0);
  idt_set_gate(13, (size_t)isr13, 0x08, IDT_INTR_PL0);
  idt_set_gate(14, (size_t)isr14, 0x08, IDT_INTR_PL0);
  idt_set_gate(15, (size_t)isr15, 0x08, IDT_INTR_PL0);
  idt_set_gate(16, (size_t)isr16, 0x08, IDT_INTR_PL0);
  idt_set_gate(17, (size_t)isr17, 0x08, IDT_INTR_PL0);
  idt_set_gate(18, (size_t)isr18, 0x08, IDT_INTR_PL0);
  idt_set_gate(19, (size_t)isr19, 0x08, IDT_INTR_PL0);
  idt_set_gate(20, (size_t)isr20, 0x08, IDT_INTR_PL0);
  idt_set_gate(21, (size_t)isr21, 0x08, IDT_INTR_PL0);
  idt_set_gate(22, (size_t)isr22, 0x08, IDT_INTR_PL0);
  idt_set_gate(23, (size_t)isr23, 0x08, IDT_INTR_PL0);
  idt_set_gate(24, (size_t)isr24, 0x08, IDT_INTR_PL0);
  idt_set_gate(25, (size_t)isr25, 0x08, IDT_INTR_PL0);
  idt_set_gate(26, (size_t)isr26, 0x08, IDT_INTR_PL0);
  idt_set_gate(27, (size_t)isr27, 0x08, IDT_INTR_PL0);
  idt_set_gate(28, (size_t)isr28, 0x08, IDT_INTR_PL0);
  idt_set_gate(29, (size_t)isr29, 0x08, IDT_INTR_PL0);
  idt_set_gate(30, (size_t)isr30, 0x08, IDT_INTR_PL0);
  idt_set_gate(31, (size_t)isr31, 0x08, IDT_INTR_PL0);
  
  idt_load((size_t)&idtp);
  printk(&state, "Pushed IDTP at 0x%8X\n", (size_t)&idtp);
}

void
set_irq_gates()
{
  idt_set_gate(32, (size_t)irq0,  0x08, IDT_INTR_PL0);
  idt_set_gate(33, (size_t)irq1,  0x08, IDT_INTR_PL0);
  idt_set_gate(34, (size_t)irq2,  0x08, IDT_INTR_PL0);
  idt_set_gate(35, (size_t)irq3,  0x08, IDT_INTR_PL0);
  idt_set_gate(36, (size_t)irq4,  0x08, IDT_INTR_PL0);
  idt_set_gate(37, (size_t)irq5,  0x08, IDT_INTR_PL0);
  idt_set_gate(38, (size_t)irq6,  0x08, IDT_INTR_PL0);
  idt_set_gate(39, (size_t)irq7,  0x08, IDT_INTR_PL0);
  idt_set_gate(40, (size_t)irq8,  0x08, IDT_INTR_PL0);
  idt_set_gate(41, (size_t)irq9,  0x08, IDT_INTR_PL0);
  idt_set_gate(42, (size_t)irq10, 0x08, IDT_INTR_PL0);
  idt_set_gate(43, (size_t)irq11, 0x08, IDT_INTR_PL0);
  idt_set_gate(44, (size_t)irq12, 0x08, IDT_INTR_PL0);
  idt_set_gate(45, (size_t)irq13, 0x08, IDT_INTR_PL0);
  idt_set_gate(46, (size_t)irq14, 0x08, IDT_INTR_PL0);
  idt_set_gate(47, (size_t)irq15, 0x08, IDT_INTR_PL0);
  idt_set_gate(48, (size_t)irqTime, 0x08, IDT_INTR_PL0);
}

const char* idt_error_desc[] =
{
  "Devide by zero Exception.",
  "Debug Exception.",
  "Non maskable interupt Exception.",
  "Breakpoint Exception.",
  "Out of bounds Exception.",
  "Invalid opcode Exception.",
  "No compressor Exception.",
  "Double fault Exception.",
  "Coprocessor Segment Overrun Exception.",
  "Bad TSS Exception.",
  "Segment not present Exception.",
  "Stack fault Exception.",
  "General protection fault Exception.",
  "Page fault Exception.",
  "Unknown interupt Exception.",
  "Compressor fault Exception.",
  "Alignment check Exception.",
  "Machine check Exception."
};

void 
idt_common_handler(const union biosregs* ireg)
{
  printk(&state, "Encountered %s halting..\n", idt_error_desc[ireg->int_no - 1]);

  //halt the system;
  halt();
}

u32
get_next_irq()
{
  i32 it = 48;

  for (; it < IDT_SIZE; it++)
  {
    if (idt[it].flags == 0 && idt[it].base_low == 0)
      return it;
  }

  return it;
}

extern void (*irq_handler_pointer)(const union biosregs *iregs);

void
subscribe_irq(u32 irq, void *handler, void *data)
{
  LOG("irq = %d\n", irq);
  idt_set_gate(irq, (size_t)irq_handler_pointer, 0x08, IDT_INTR_PL0);

  irq_handlers[irq].data = data;
  irq_handlers[irq].callback = handler;
}

void
idt_call_irq(u32 irq, const union biosregs *iregs)
{
  irq_handlers[irq].callback(iregs, irq_handlers[irq].data);
}