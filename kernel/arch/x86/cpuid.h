#ifndef __CPIID_H__
#define __CPUID_H__

#include <common.h>
//return vendor string in 4 registers
//ex EBX["Genu"] ECX["ntel"] EDX["ineI"]
#define CPUID_GET_VENDOR      0x0 

//returns
//EAX Version Information: Type, Family, Model, and Stepping ID
//EBX Brand Index. CLFLUSH line size, Maximum number of accessable IDs, Initial APIC ID
//ECX Feature information see below
//EDX Feature information see below
#define CPUID_GET_FEATURES    0x1 

//returns cache and TLB information
#define CPUID_GET_LB          0x2

//returns
//EAX reserved
//EBX reserved
//ECX Processor serial number 00-31 bit
//EDX Processor serial number 32-63 bit
#define CPUID_GET_SERIAL      0x3

//returns
// Maximum Input Value for Extended Function CPUID Information.
#define INTEL_EXTENDED        0x80000000

//returns
//EAX Extended Processor Signature and Feature Bits.
//EBX reserved
// EBX Reserved.
// ECX Bit  00: LAHF/SAHF available in 64-bit mode.
//     Bits 04 - 01: Reserved.
//     Bit  05: LZCNT.
//     Bits 07 - 06: Reserved.
//     Bit  08: PREFETCHW.
//     Bits 31 - 09: Reserved.
// EDX Bits 10 - 00: Reserved.
//     Bit  11: SYSCALL/SYSRET available in 64-bit mode.
//     Bits 19 - 12: Reserved = 0.
//     Bit  20: Execute Disable Bit available.
//     Bits 25 - 21: Reserved = 0.
//     Bit  26: 1-GByte pages are available if 1.
//     Bit  27: RDTSCP and IA32_TSC_AUX are available if 1.
//     Bit  28: Reserved = 0.
//     Bit  29: Intel® 64 Architecture available if 1.
//     Bits 31 - 30: Reserved = 0.
#define INTEL_FEATURES        0x80000001

//returns continious Processor brand string ecx, ebx, ecx, edx loop.
#define INTEL_BRANDSTRING     0x80000002
#define INTEL_BRANDSTRINGMORE 0x80000003
#define INTEL_BRANDSTRINGEND  0x80000004

//NOTE: See 64 ia32 Architectures Software Developer Instruction Set Reference Manual Vol.2A 3-206
//
//EAX[]


//NOTE: See 64 ia32 Architectures Software Developer Instruction Set Reference Manual Vol.2A 3-206
#define CPUID_FEAT_ECX_SSE3         (1 <<  0) //SSE3 support
#define CPUID_FEAT_ECX_PCLMUL       (1 <<  1) //PCLMULQDQ support
#define CPUID_FEAT_ECX_DTES64       (1 <<  2) //64-bit debug store
#define CPUID_FEAT_ECX_MONITOR      (1 <<  3) //MONITOR and NWAIT instruction (sse3)
#define CPUID_FEAT_ECX_DS_CPL       (1 <<  4) //CPL qualified debug store
#define CPUID_FEAT_ECX_VMX          (1 <<  5) //Virtual Machin Extensions
#define CPUID_FEAT_ECX_SMX          (1 <<  6) //Safer Mode Extensions (intel(R) Trusted Execution Technology)
#define CPUID_FEAT_ECX_EST          (1 <<  7) //Enachanced intel SpeedTest Technology (IA32_PERF_STS, IA32_PERF_CTL registers)
#define CPUID_FEAT_ECX_TM2          (1 <<  8) //Thermal monitor 2(Thermal Circuit Controll TCC)
#define CPUID_FEAT_ECX_SSSE3        (1 <<  9) //Suplemental Streaming SIMD Extension 3
#define CPUID_FEAT_ECX_CNTX_ID      (1 << 10) //L1 Context ID (The L1 data cache mode can be set by the BIOS)
#define CPUID_FEAT_ECX_CX16         (1 << 13) //CMPXCHG16B instruction support
#define CPUID_FEAT_ECX_XTPR         (1 << 14) //xTPR Update Controll
#define CPUID_FEAT_ECX_PDCM         (1 << 15) //Perform and Debug Capability
#define CPUID_FEAT_ECX_PCID         (1 << 17) //Process Context Identifiers
#define CPUID_FEAT_ECX_DCA          (1 << 18) //Prefatch data from memory mapped device
#define CPUID_FEAT_ECX_SSE41        (1 << 19) //Supprots SSE4.1 instruction set.
#define CPUID_FEAT_ECX_SSE42        (1 << 20) //Supprots SSE4.2 instruction set.
#define CPUID_FEAT_ECX_X2APIC       (1 << 21) //Supports x2APIC feature.
#define CPUID_FEAT_ECX_MOVBE        (1 << 22) //Supprots MOVBE instruction.
#define CPUID_FEAT_ECX_POPCNT       (1 << 23) //Supports POPCNT instruction.
#define CPUID_FEAT_ECX_TSCDEADLINE  (1 << 24) //Local APIC timer supports one-shot operation using TSC deadline value.
#define CPUID_FEAT_ECX_AESNI        (1 << 25) //Supports AESNI Instruction Set.
#define CPUID_FEAT_ECX_XSAVE        (1 << 26) //Supports XSAVE/XRSTOR processor extended states feature, the XSETBV/XGETBV instructions, and XCR0. 
#define CPUID_FEAT_ECX_OSXSAVE      (1 << 27) //A value of 1 indicates that the OS has set CR4.OSXSAVE[bit 18] to enable XSETBV/XGETBV
                                              //instructions to access XCR0 and to support processor extended state management using
                                              //XSAVE/XRSTOR.
#define CPUID_FEAT_ECX_AVX          (1 << 28) //Supports AVX instruction set.
#define CPUID_FEAT_ECX_F16C         (1 << 29) //Supports 16 bit float conversion instructions
#define CPUID_FEAT_RDRAND           (1 << 30) //Supports RDRAND instructions.

//NOTE: See 64 ia32 Architectures Software Developer Instruction Set Reference Manual Vol.2A 3-209
#define CPUID_FEAT_EDX_FPU          (1 <<  0) //Floating Point Unit On-Chip
#define CPUID_FEAT_EDX_VME          (1 <<  1) //Virtual 8086 Mode Enhancements.
#define CPUID_FEAT_EDX_DE           (1 <<  2) //Debugging Extension
#define CPUID_FEAT_EDX_PSE          (1 <<  3) //Page Size Extension. 4MB page sizes
#define CPUID_FEAT_EDX_TSC          (1 <<  4) //Time Stamp Conunter
#define CPUID_FEAT_EDX_MSR          (1 <<  5) //Model Specific registers.
#define CPUID_FEAT_EDX_PAE          (1 <<  6) //Physical Adress Extension.
#define CPUID_FEAT_EDX_MCE          (1 <<  7) //Machine Check Exception.
#define CPUID_FEAT_EDX_CX8          (1 <<  8) //CMPXCHG8B instruction
#define CPUID_FEAT_EDX_APIC         (1 <<  8) //APIC on-chip
#define CPUID_FEAT_EDX_SEP          (1 << 11) //SYSENTER and SYSEXIT Instructions.
#define CPUID_FEAT_EDX_MTRR         (1 << 12) //Memory Type Range Registers.
#define CPUID_FEAT_EDX_PGE          (1 << 13) //Page Global Bit
#define CPUID_FEAT_EDX_MCA          (1 << 14) //Machine Check Architecture.
#define CPUID_FEAT_EDX_CMOV         (1 << 15) //Conditional Move instructions.
#define CPUID_FEAT_EDX_PAT          (1 << 16) //Page Attribute Table.
#define CPUID_FEAT_EDX_PSE36        (1 << 17) //36 Bit Page Size Extension.
#define CPUID_FEAT_EDX_PSN          (1 << 18) //Processor Serial Number.
#define CPUID_FEAT_EDX_CLFSH        (1 << 19) //CLFLUSH instructions.
#define CPUID_FEAT_EDX_DS           (1 << 21) //Debug store
#define CPUID_FEAT_EDX_ACPI         (1 << 22) //Thermal Monitor and Software Controlled Clock Facalities.
#define CPUID_FEAT_EDX_MMX          (1 << 23) //Intel MMX Technology.
#define CPUID_FEAT_EDX_FXSR         (1 << 24) //FXSAVE and FXRESTORE Instructions.
#define CPUID_FEAT_EDX_SSE          (1 << 25) //Supports SSE extensions.
#define CPUID_FEAT_EDX_SSE2         (1 << 26) //Supports SSE2 Extensions.
#define CPUID_FEAT_EDX_SS           (1 << 27) //Self snoop
#define CPUID_FEAT_EDX_HIT          (1 << 28) //Max APIC IDs reserved field is Valid.
#define CPUID_FEAT_EDX_TM           (1 << 29) //Thermal Monitor.
#define CPUID_FEAT_EDX_PBE          (1 << 30) //Pending Break Enable.

void
cpuid(i32 code, u32 *a, u32 *b, u32 *c, u32 *d);

void
cpuid_string(i32 code, i8 *buffer);

#endif