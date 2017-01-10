#include <common.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/irq.h>
#include <arch/x86/io.h>
#include <arch/x86/page.h>

#include <keyboard.h>
#include <memory.h>


extern uint32 end_kernel;

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

  map_page((void*)0x12FF0000, (void*)0x12340000, 0x3);
  uint32 addr = (uint32)get_physaddr((void*)0x12340000);

  printk(&state, "Phys addr = %8X\n", addr);
}
