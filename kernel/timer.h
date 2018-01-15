#ifndef __TIMER_H__
#define __TIMER_H__

#include <common.h>
#include <list.h>

typedef void(*timer_function_proc)(u32);

//timer queue in nanoseconds
struct timer_list_entry
{
  u32 timer;
  timer_function_proc function_callback;
  u32 callback_arg;

  struct slist_node node;
};

void
timer_init();

void
new_timer(u32 time, timer_function_proc function, u32 callback_arg);

void
sleep(i32 ms);

#endif
