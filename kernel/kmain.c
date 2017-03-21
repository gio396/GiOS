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
#include <arch/x86/pit.h>

#include <timer.h>
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
  cpuid(CPUID_GET_FEATURES, &eax, &ebx, &ecx, &edx);

  irq_install();
  pit_init();

  if((edx & CPUID_FEAT_EDX_APIC))
  {
    apic_enable(APIC_LOCATION);

    //Save previous masks
    get_interrupt_masks(&mask1, &mask2);

    //Mask all the PIC interrupts
    set_interrupt_masks(0xFE, 0xFE);

    timer_init(16, apic_timer_interrupt_in, apic_timer_get_count);
  }
  else
  {
    timer_init(0, pit_interrupt_in, pit_get_current_count);
  }

  keyboard_install(0);
  apic_init_timer();

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


  struct timer_list_entry entry;

  for (int i = 0; i < 10; i++)
  {
    entry.timer = (i + 1) * 1000000;
    entry.op_code = 0;

    queue_add_timer(entry);
  }
}
