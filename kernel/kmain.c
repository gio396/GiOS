#include <common.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/irq.h>
#include <arch/x86/io.h>
#include <arch/x86/page.h>

#include <keyboard.h>
#include <memory.h>
#include <assert.h>
#include <macros.h>

extern const uint32 l_srodata;
extern const uint32 l_erodata;
extern const uint32 l_sdata;
extern const uint32 l_edata;
extern const uint32 l_sbss;
extern const uint32 l_ebss;
extern const uint32 l_ekernel;

void 
kmain()
{
  terminal_init(&state);
  printk(&state, "Init done\n");

  gdt_install();
  idt_install();
  irq_install();
  keyboard_install(0);

  //have to change this
  // page_init();

  printk(&state, "\n");
  printk(&state, "read only data [0x%8X, 0x%8X]\n", &l_srodata, &l_erodata);
  printk(&state, "data           [0x%8X, 0x%8X]\n", &l_sdata, &l_edata);
  printk(&state, "bss            [0x%8X, 0x%8X]\n", &l_sbss, &l_ebss);
  printk(&state, "end of kernel  0x%8X\n", &l_ekernel);
  printk(&state, "\n"); 
}