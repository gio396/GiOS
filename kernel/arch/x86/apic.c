#include "apic.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/msr.h>

#define IA32_APIC_BASE_MSR        0x1B
//63      MAXPHYSADDR       11  10   8    7    0
//[R--------*-------|BASE-32-|GE-|R--|BSP-|R-7-]

#define IA32_APIC_BASE_MSR_BSP    0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

void
apic_write_reg(uint32 reg, uint32 val)
{
  uint32 offset = reg;
  uint32 volatile *reg_addr = (uint32*)(APIC_LOCATION + offset);

  reg_addr[0] = val;
}

uint32
apic_read_reg(uint32 reg)
{
  uint32 offset = reg;
  uint32 *reg_addr = (uint32*)(APIC_LOCATION + offset);

  return(*reg_addr);
}

void
apic_enable(uint32 apic_base_address)
{
  (void)(apic_base_address);
  uint32 apic_base_register_lo;
  uint32 apic_base_register_hi;

  apic_base_register_lo = (apic_base_address & 0xFFFFF000) | IA32_APIC_BASE_MSR_ENABLE;
  apic_base_register_hi = 0;

  cpu_set_msr(IA32_APIC_BASE_MSR, apic_base_register_hi, apic_base_register_lo);

  /* Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts */

  uint32 spurious_int_vec = apic_read_reg(APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER);


  printk(&state, "SIV %32b\n", spurious_int_vec);
  printk(&state, "SIV %32b\n", spurious_int_vec | 0x100);

  apic_write_reg(APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER, 
                 apic_read_reg(APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER) | 0x100
                );
}


