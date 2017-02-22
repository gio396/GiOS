#ifndef __TIMER_H__
#define __TIMER_H__

#include <common.h>
#include <list.h>

#define TIMER_MAX 0 
//apic 4.29 seconds

#define OP_CODE_PRINT 0x00

//timer queue in nanoseconds
struct timer_list_entry
{
  uint32 timer;
  uint8  op_code;

  struct slist_node node;
};

void
timer_init();

void
queue_add_timer(struct timer_list_entry new_entry);

#endif
