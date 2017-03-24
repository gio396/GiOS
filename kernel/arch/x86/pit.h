#ifndef __PIT_H__
#define __PIT_H__

#include <common.h>

#define INTERUPT_MAX             0 //55.07 milliseconds
#define PIT_DEF_FREQUENCY        1193182

void
pit_init();

void
pit_system_timer_init();

void
pit_interrupt_in(size_t time);

uint16
pit_get_current_count(void);

#endif