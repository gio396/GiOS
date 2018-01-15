#include "msr.h"

#include <arch/x86/cpuid.h>

b32
cpu_has_msr()
{
  u32 a, b, c, d;

  cpuid(CPUID_GET_FEATURES, &a, &b, &c, &d);

  return (d & CPUID_FEAT_EDX_MSR);
}

void
cpu_get_msr(u32 msr, u32 *lo, u32 *hi)
{
  __asm__ __volatile__ ("rdmsr": "=a"(*lo), "=d"(*hi): "c"(msr));
}

u64
cpu_get_msr_u64(u32 msr)
{
  u64 res;
  u32 *lo = (u32*)&res;

  cpu_get_msr(msr, lo + 1, lo);

  return res;
}

void
cpu_set_msr(u32 msr, u32 lo, u32 hi)
{
  __asm__ __volatile__ ("wrmsr"::"a"(lo), "b"(hi), "c"(msr));
}