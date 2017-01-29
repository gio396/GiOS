#include <common.h>

#include <arch/x86/framebuffer.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/irq.h>
#include <arch/x86/io.h>
#include <arch/x86/page.h>

#include <keyboard.h>
#include <mboot_header.h>

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
kmain(uint32 mboot_magic, struct multiboot_info *mboot_info)
{
  (void)(mboot_magic);
  (void)(mboot_info);

  terminal_init(&state);
  printk(&state, "Init done\n");

  //check boot magic number.
  if (mboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    printk(&state, "Error wrong multiboot magic number. halting...\n");
    return;    
  }

  page_init();  
  gdt_install();
  idt_install();
  irq_install();
  keyboard_install(0);

  printk(&state, "\n");
  printk(&state, "read only data [0x%8X, 0x%8X]\n", &l_srodata, &l_erodata);
  printk(&state, "data           [0x%8X, 0x%8X]\n", &l_sdata, &l_edata);
  printk(&state, "bss            [0x%8X, 0x%8X]\n", &l_sbss, &l_ebss);
  printk(&state, "end of kernel  0x%8X\n", &l_ekernel);
  printk(&state, "\n"); 

}
