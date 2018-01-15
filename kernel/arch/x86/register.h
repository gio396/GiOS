#ifndef __REGISTER_H__
#define __REGISTER_H__

#include <common.h>

union biosregs
{
  // for interupts;
  struct 
  {
    u32 i_gs, i_fs, i_es, i_ds;
    u32 i_edi, i_esi, i_ebp, i_esp, i_ebx, i_edx, i_ecx, i_eax; //pusha
    u32 int_no, err_code;
    u32 i_eip, i_cs, i_eflags, i_useresp, i_ss;
  };

  //general
  struct
  {
    u32 gs, fs, es, ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; //pusha
    u32 _fsgs, _dses, eflags;
  };
};

//no flags or gs, fs, es, ds
#define REG_PUSH(r)\
  __asm__ __volatile__ ("":: "a"((r).eax), "b"((r).ebx), "c"((r).ecx), "d"((r).edx), "S"((r).esi), "D"((r).edi))\

#define REG_POP(r)\
  __asm__ __volatile__ ("": "=a"((r).eax), "=b"((r).ebx), "=c"((r).ecx), "=d"((r).edx), "=S"((r).esi), "=D"((r).edi))\

#endif