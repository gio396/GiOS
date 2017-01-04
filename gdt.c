#include "gdt.h"

#include "common.h"
#include "framebuffer.h"

internal void
gdt_set_gate(int32 num, uint32 base, uint32 limit, uint8 access, uint8 gran)
{
  printk(&state, "seting gdt gate %d, base %d, limit %d, access %d, gran %d\n",
         num, base, (int32)limit, access, gran);

  gdt[num].base_low =    (base & 0xFFFF);
  gdt[num].base_middle = ((base >> 16) & 0xFF);
  gdt[num].base_high =   ((base >> 24) & 0xFF);


  gdt[num].limit_low =   (limit & 0xFFFF);
  gdt[num].granularity = ((limit >> 16) & 0x0F);

  gdt[num].granularity |= (gran & 0xF0);
  gdt[num].access = access;
}

void 
gdt_install()
{
  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base = (int32)&gdt;

  printk(&state, "gp limit %d, gp base %d\n",
                      gp.limit, gp.base);

  // null descriptor
  gdt_set_gate(0,0,0,0,0);

  //code segment 4gb, 4kb aligned
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

  //data segment 4gb, 4kb aligned
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

  printk(&state, "flushed gp at %d\n", (int32)&gp);
  gdt_flush((int32)&gp);
  printk(&state, "flushed gp at %d\n", (int32)&gp);

}