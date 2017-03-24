#include "pit.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/io.h>
#include <arch/x86/irq.h>

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_3 0x42
#define PIT_MODE_COM  0x43

#define OP_MODE_0 (0x0 << 1)  //Interrupt on terminal count.
#define OP_MODE_1 (0x1 << 1)  //Hardware re-triggerable one-shot
#define OP_MODE_2 (0x2 << 1)  //Rate generator
#define OP_MODE_3 (0x3 << 1)  //Square wave generator
#define OP_MODE_4 (0x4 << 1)  //Software triggered strobe
#define OP_MODE_5 (0x5 << 1)  //Hardware triggered strobe

#define PIT_SEG_BCD(x)  (x)
#define PIT_SEG_ACC(x)  ((x) << 4)
#define PIT_SEG_CHN(x)  ((x) << 6)

#define NANOSECONDS_IN_COUNT 840
#define MAX_ONE_SHOT         65536

//COM_MODE
//8      6          4          1        0
//[CHAN--|ACC_MODE--|OP_MODE---|BCD/BIN-|]
//
//
//CHAN select channel
//    0 0 - channel 0
//    0 1 - channel 1
//    1 0 - channel 2
//    1 1 - read_back

//ACC_MODE access mode
//    0 0 - latch count value
//    0 1 - lobyte only
//    1 0 - hibyte only
//    1 1 - lobyte/hibite mode
//    will wait for lo byte than higbyte on channel

//OP_MODE Operating mode one of the 5 defined operating modes.

//BCD/BIN BCD/binary mode
//    0   - 16-bit binary mode
//    1   - Four digit BCD 


void
pit_system_timer_handler(/*regs*/)
{
  static int cnt = 0;

  if ((cnt++ % 1000) == 0 )
  {
    printk(&state, "second\n");
  } 
}

void
pit_init(void)
{
  uint8 mode = PIT_SEG_CHN(0) | OP_MODE_1 | PIT_SEG_ACC(0x3) | PIT_SEG_BCD(0);
  outb(PIT_MODE_COM, mode);
  outb(PIT_CHANNEL_0, 0);
  outb(PIT_CHANNEL_0, 0);
}

void
pit_system_timer_init()
{
  uint8 mode = PIT_SEG_CHN(0) | OP_MODE_3 | PIT_SEG_ACC(0x3) | PIT_SEG_BCD(0);
  outb(PIT_MODE_COM, mode);
  outb(PIT_CHANNEL_0, 0);
  outb(PIT_CHANNEL_0, 0);
  pit_interrupt_in(1193);

  irq_set_handler(0, pit_system_timer_handler);
}

void
pit_interrupt_in(size_t time)
{
  disable_interrupts();

  uint8 lo;
  uint8 hi;

  time = time & 0x0000FFFF;

  lo = time & 0xFF;
  hi = (time >> 8) & 0xFF;

  outb(PIT_CHANNEL_0, lo);
  outb(PIT_CHANNEL_0, hi);

  enable_interrupts();
}

uint16
pit_get_current_count(void)
{
  disable_interrupts();

  uint8 mode = 0;
  uint16 lo, hi = 0;

  outb(PIT_MODE_COM, mode);
  lo = inb(PIT_CHANNEL_0);
  hi = inb(PIT_CHANNEL_0);

  enable_interrupts();
  return ((hi << 8) | lo);
}
