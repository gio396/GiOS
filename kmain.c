#include "common.h"
#include "framebuffer.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "memory.h"
#include "io.h"
#include "keyboard.h"
#include "page.h"

void 
kstart()
{
  terminal_init(&state);
  printk(&state, "Init done\n");

  gdt_install();
  idt_install();
  irq_install();
  keyboard_install(0);

  page_init();

  for(;;)
    halt();
}
