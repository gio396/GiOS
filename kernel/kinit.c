#include "kinit.h"

#include <arch/x86/framebuffer.h>

#define LINKER_VAR(vname) __l_##vname
#define DEF_LINKER_VAR(vname) extern u32 LINKER_VAR(vname);

DEF_LINKER_VAR(init_core0_s);

void
kinit_core0()
{
  u32 core0_size = *(u32*)&LINKER_VAR(init_core0_s);
  u32 *core_list = (u32*)(&LINKER_VAR(init_core0_s) + 1);

  for (u32 i = 0; i < core0_size; i++)
  {
    void (*fptr)(void) = (void(*)(void))core_list[i];
    if (fptr != NULL)
    {
      fptr();
    }
  }
}