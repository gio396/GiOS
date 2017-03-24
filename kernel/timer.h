#ifndef __TIMER_H__
#define __TIMER_H__

#include <common.h>
#include <list.h>

#define TIMER_MAX 0 
//apic 4.29 seconds

#define OP_CODE_PRINT 0x00

typedef void(*timer_function_proc)(uint32);

//timer queue in nanoseconds
struct timer_list_entry
{
  uint32 timer;
  timer_function_proc function_callback;
  uint32 callback_arg;

  struct slist_node node;
};

void
timer_init();

void
new_timer(uint32 time, timer_function_proc function, uint32 callback_arg);

#endif
