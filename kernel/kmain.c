#include <common.h>

#include <arch/x86/framebuffer.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>
#include <arch/x86/irq.h>
#include <arch/x86/io.h>
#include <arch/x86/page.h>
#include <arch/x86/cpuid.h>
#include <arch/x86/apic.h>
#include <arch/x86/acpi.h>

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
  (void)(mboot_info);
  uint8 mask1, mask2;
  uint32 eax, ebx, edx, ecx;

  cpuid(CPUID_GET_FEATURES, &eax, &ebx, &ecx, &edx);

  // #ifdef QEMU_DBG
  init_serial();
  // #endif

  terminal_init(&state);
  printk(&state, "Init done\n");

  //check boot magic number.
  if (mboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    printk(&state, "Error wrong multiboot magic number. halting...\n");

    halt();    
  }

  page_init();  
  gdt_install();
  idt_install();
  find_rsdp();

  if((edx & CPUID_FEAT_EDX_APIC))
  {
    apic_enable(APIC_LOCATION);

    //Save previous masks
    get_interrupt_masks(&mask1, &mask2);

    //Mask all the PIC interrupts
    // set_interrupt_masks(0xFF, 0xFF);
  }  

  irq_install();  

  keyboard_install(0);

  printk(&state, "\n");
  printk(&state, "read only data [0x%8X, 0x%8X]\n", &l_srodata, &l_erodata);
  printk(&state, "data           [0x%8X, 0x%8X]\n", &l_sdata, &l_edata);
  printk(&state, "bss            [0x%8X, 0x%8X]\n", &l_sbss, &l_ebss);
  printk(&state, "end of kernel  0x%8X\n", &l_ekernel);
  printk(&state, "\n");

  int8 buffer[17];
  buffer[16] = '\0';
  cpuid_string(CPUID_GET_VENDOR, buffer);

  printk(&state, "CPU vendor: %s\n", buffer + 4);

  void *apic_table = find_rstd_descriptor(RSDT_APIC);

  printk(&state, "APIC table %08X\n", apic_table);
}
