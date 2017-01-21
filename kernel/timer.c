#include "timer.h"

#include <arch/x86/irq.h>
#include <arch/x86/framebuffer.h>

#include <arch/x86/register.h>

#define PIT_CHANNEL0_DATA 0x40
#define PIT_CHANNEL1_DATA 0x41
#define PIT_CHANNEL2_DATA 0x42
#define PID_MODE_COMMANd  0x43

//mode command register contents
// 8    6    4     1    0
// [SC--|AM--|OM---|BCD-]



//TODO(GIO): usa APIC
void
system_timer_handler(/*const union biosregs* iregs*/)
{
  // terminal_flash(&state, 10);
}

void
init_system_timer(uint32 rate)
{
  //TODO(gio396) set system timer rate;
  (void)(rate);

  irq_set_handler(0, system_timer_handler);
}