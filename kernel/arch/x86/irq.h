#ifndef __IRQ_H__
#define __IRQ_H__

#include "register.h"
#include "common.h"

#define NO_INT(code)    \
{                       \
  dissable_interrupts();\
  code                  \
  enable_interrupts();  \
}                       \

void
irq_common_handler(const union biosregs* ireg);

void
irq_install(void);

void
irq_set_handler(u8 num, void* handler);

void
irq_clear_handler(u8 num);

void
set_interrupt_masks(u8 mask1, u8 mask2);

void
get_interrupt_masks(u8 *mask1, u8 *mask2);

void
disable_interrupts(void);

void
enable_interrupts(void);


#endif