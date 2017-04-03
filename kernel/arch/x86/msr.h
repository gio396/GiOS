#ifndef __MSR_H__
#define __MSR_H__

#include <common.h>

//63          16
//[ reserved  |CS selector]
#define IA32_SYSENTER_CS_MSR    0x174

#define IA32_SYSENTER_ESP_MSR  0x175
#define IA32_SYSENTER_EIP_MSR  0x176
#define IA32_APIC_BASE_MSR      0x1B

b32
cpu_has_msr();

void 
cpu_get_msr(uint32 msr, uint32 *lo, uint32 *hi);

uint64
cpu_get_msr_uint64(uint32 msr);

void
cpu_set_msr(uint32 msr, uint32 lo, uint32 hi);

#endif
