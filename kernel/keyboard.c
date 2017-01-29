#include "keyboard.h"

#include <arch/x86/register.h>
#include <arch/x86/io.h>
#include <arch/x86/irq.h>
#include <arch/x86/framebuffer.h>

#include <macros.h>

unsigned char kbdlt[128] =
{
   0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',     /* Tab */
  'q', 'w', 'e', 'r', /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
  0,      /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
  '\'', '`',   0,    /* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n',      /* 49 */
   'm', ',', '.', '/',   0,        /* Right shift */
   '*',
    0,  /* Alt */
   ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    67,  /* Up Arrow */
    0,  /* Page Up */
   '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
   '+',
    0,  /* 79 - End key*/
    68,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};


internal void
keyboard_handler(/*const union biosregs *ireg*/)
{
  uint8 scancode;

  scancode = inb(0x60);

  if (scancode & 0x80) //released
  {

  }
  else
  {
    switch(kbdlt[scancode])
    {
      CASE(
        terminal_load_prev();
      , 67);

      CASE(
        terminal_load_next();
      , 68);

      default:
        terminal_put_char(&state, kbdlt[scancode]);
    }
  }
}

void
keyboard_install(uint16 refresh_rate)
{
  (void)(refresh_rate);
  irq_set_handler(1, keyboard_handler);
}