#include "idt.h"

#include "memory.h"
#include "framebuffer.h"
#include "register.h"


internal void 
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

  idt_set_gate(0, (uint32)isr0, 0x08, 0x8E);

  idt_load((int32)&idtp);
  printk(&state, "Pushed idtp at %d\n", (int32)&idtp);
}


void 
isr0_handler(const struct ireg* reg)
{
  (void)(reg);
  terminal_put_string(&state, "ZERO EXCEPTION HANDLER\n");
}