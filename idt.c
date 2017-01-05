#include "idt.h"

#include "memory.h"
#include "framebuffer.h"
#include "register.h"
#include "io.h"

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

void 
idt_set_gate(int32 num, uint32 base, uint16 sel, uint8 flags)
{
  printk(&state, "Setting idt gate %d at base %d, sle %d, flags %d\n",
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
  printk(&state, "Installing idt table\n");

  idtp.limit = (sizeof (struct idt_entry) * IDT_SIZE) - 1;
  idtp.base = (int32)&idt;

  printk(&state, "sizeof idt_entry %d\n", sizeof(struct idt_entry));
  printk(&state, "idtp limit %d, base %d\n", idtp.limit, idtp.base);

  memset (&idt, 0, sizeof (struct idt_entry) * 256);

  idt_set_gate(0,  (uint32)isr0,  0x08, 0x8E);
  idt_set_gate(1,  (uint32)isr1,  0x08, 0x8E);
  idt_set_gate(2,  (uint32)isr2,  0x08, 0x8E);
  idt_set_gate(3,  (uint32)isr3,  0x08, 0x8E);
  idt_set_gate(4,  (uint32)isr4,  0x08, 0x8E);
  idt_set_gate(5,  (uint32)isr5,  0x08, 0x8E);
  idt_set_gate(6,  (uint32)isr6,  0x08, 0x8E);
  idt_set_gate(7,  (uint32)isr7,  0x08, 0x8E);
  idt_set_gate(8,  (uint32)isr8,  0x08, 0x8E);
  idt_set_gate(9,  (uint32)isr9,  0x08, 0x8E);
  idt_set_gate(10, (uint32)isr10, 0x08, 0x8E);
  idt_set_gate(11, (uint32)isr11, 0x08, 0x8E);
  idt_set_gate(12, (uint32)isr12, 0x08, 0x8E);
  idt_set_gate(13, (uint32)isr13, 0x08, 0x8E);
  idt_set_gate(14, (uint32)isr14, 0x08, 0x8E);
  idt_set_gate(15, (uint32)isr15, 0x08, 0x8E);
  idt_set_gate(16, (uint32)isr16, 0x08, 0x8E);
  idt_set_gate(17, (uint32)isr17, 0x08, 0x8E);
  idt_set_gate(18, (uint32)isr18, 0x08, 0x8E);
  idt_set_gate(19, (uint32)isr19, 0x08, 0x8E);
  idt_set_gate(20, (uint32)isr20, 0x08, 0x8E);
  idt_set_gate(21, (uint32)isr21, 0x08, 0x8E);
  idt_set_gate(22, (uint32)isr22, 0x08, 0x8E);
  idt_set_gate(23, (uint32)isr23, 0x08, 0x8E);
  idt_set_gate(24, (uint32)isr24, 0x08, 0x8E);
  idt_set_gate(25, (uint32)isr25, 0x08, 0x8E);
  idt_set_gate(26, (uint32)isr26, 0x08, 0x8E);
  idt_set_gate(27, (uint32)isr27, 0x08, 0x8E);
  idt_set_gate(28, (uint32)isr28, 0x08, 0x8E);
  idt_set_gate(29, (uint32)isr29, 0x08, 0x8E);
  idt_set_gate(30, (uint32)isr30, 0x08, 0x8E);
  idt_set_gate(31, (uint32)isr31, 0x08, 0x8E);
  
  idt_load((int32)&idtp);
  printk(&state, "Pushed idtp at %d\n", (int32)&idtp);
}

const char* idt_error_desc[] =
{
  "Devide by zero Exception.",
  "Debug Exception.",
  "Non maskable interupt Exception.",
  "Breakpoint Exception.",
  "Into detected overflow Exception.",
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
  printk(&state, "Encountered %s halting..\n", idt_error_desc[ireg->int_no]);

  //halt the system;
  halt();
}