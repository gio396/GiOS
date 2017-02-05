#include "msr.h"

#include <arch/x86/cpuid.h>

b32
cpu_has_msr()
{
  uint32 a, b, c, d;

  cpuid(CPUID_GET_FEATURES, &a, &b, &c, &d);

  return (d & CPUID_FEAT_EDX_MSR);
}

void
cpu_get_msr(uint32 msr, uint32 *lo, uint32 *hi)
{
  __asm__ __volatile__ ("rdmsr": "=a"(*lo), "=d"(*hi): "c"(msr));
}

uint64
cpu_get_msr_uint64(uint32 msr)
{
  uint64 res;
  uint32 *lo = (uint32*)&res;

  cpu_get_msr(msr, lo + 1, lo);

  return res;
}

void
cpu_set_msr(uint32 msr, uint32 lo, uint32 hi)
{
  __asm__ __volatile__ ("wrmsr"::"a"(lo), "b"(hi), "c"(msr));
}