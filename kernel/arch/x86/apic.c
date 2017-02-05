#include "apic.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/msr.h>

#define APIC_LOCATION             0xFEC00000
#define IA32_APIC_BASE_MSR_BSP    0x1B
#define IA32_APIC_BASE_MSR_BSP    0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

void
apic_enable()
{
  uint32 apic_base_register_lo, apic_base_register_hi;

  cpu_get_msr(IA32_APIC_BASE_MSR, &apic_base_register_lo, &apic_base_register_hi);

  printk(&state, "ia32_apic_base_register hi %32b lo %32b\n", apic_base_register_hi, apic_base_register_lo);

  return;
} 