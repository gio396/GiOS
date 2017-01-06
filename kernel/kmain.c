#include <common.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/irq.h>
#include <arch/x86/io.h>
#include <arch/x86/page.h>

#include <keyboard.h>
#include <memory.h>

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
}
