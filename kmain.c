#include "common.h"
#include "framebuffer.h"
#include "gdt.h"
#include "idt.h"
#include "memory.h"

void 
kstart()
{
  terminal_init(&state);
  printk(&state, "Init done\n");

  printk(&state, "isr0 bgdt %d\n", (int32)isr0);

  gdt_install();
  idt_install();
  enable_pmode();

  int pmode = check_pmode(); 

  printk(&state, "Protected mode is %s\n", (pmode == 0 ? "off":"on"));

  int a = 0;
  int b = 1;

  int f = b/a;
  printk(&state, "f %d\n", f);
}
