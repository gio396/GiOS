#ifndef __MSR_H__
#define __MSR_H__

#include <common.h>

//63          16
//[ reserved  |CS selector]
#define IA32_SYSENTER_CS_MSR    0x174

#define IA32_SYSENTER_ESP_MSR  0x175
#define IA32_SYSENTER_EIP_MSR  0x176
#define IA32_APIC_BASE_MSR      0x1B

#define IA32_STAR_MSR         0xC0000081
#define IA32_LSTAR_MSR        0xC0000082
#define IA32_CSTAR_MSR        0xC0000083
#define IA32_SFMASK_MSR       0xC0000084

b32
cpu_has_msr();

void 
cpu_get_msr(u32 msr, u32 *lo, u32 *hi);

u64
cpu_get_msr_u64(u32 msr);

void
cpu_set_msr(u32 msr, u32 lo, u32 hi);

#endif
