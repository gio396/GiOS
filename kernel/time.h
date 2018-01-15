//pit system time

#ifndef __TIME_H__
#define __TIME_H__

#include <common.h>

void
init_pit_system_timer(void);

u32 
get_system_time_in_ms(void);

u32
get_system_time_in_s(void);

#endif //header guard
