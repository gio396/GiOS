#include "time.h"

#include <arch/x86/pit.h>
#include <arch/x86/irq.h> 

global u32 start_time;

void
time_handler()
{
  start_time++;
}

void
init_pit_system_timer(void)
{
  pit_system_timer_init();

  irq_set_handler(0, time_handler);
}

u32
get_system_time_in_ms(void)
{
  return start_time;
}

u32
get_system_time_in_s(void)
{
  return start_time / 1000;
}
