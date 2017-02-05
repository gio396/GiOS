#ifndef __MSR_H__
#define __MSR_H__

#include <common.h>

b32
cpu_has_msr();

void 
cpu_get_msr(uint32 msr, uint32 *lo, uint32 *hi);

uint64
cpu_get_msr_uint64(uint32 msr);

void
cpu_set_msr(uint32 msr, uint32 lo, uint32 hi);

#endif
