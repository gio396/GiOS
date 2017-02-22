#ifndef __PIT_H__
#define __PIT_H__

#include <common.h>

#define INTERUPT_MAX 0 //55.07 milliseconds

void
pit_init();

//in usecs
void
pit_interrupt_in(size_t time);

uint16
pit_get_current_count();

#endif