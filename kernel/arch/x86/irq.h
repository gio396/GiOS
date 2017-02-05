#ifndef __IRQ_H__
#define __IRQ_H__

#include "register.h"
#include "common.h"

void
irq_common_handler(const union biosregs* ireg);

void
irq_install(void);

void
irq_set_handler(uint8 num, void* handler);

void
irq_clear_handler(uint8 num);

void
set_interrupt_masks(uint8 mask1, uint8 mask2);

void
get_interrupt_masks(uint8 *mask1, uint8 *mask2);

#endif