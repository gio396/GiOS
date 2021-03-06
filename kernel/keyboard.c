#include "keyboard.h"

#include <arch/x86/register.h>
#include <arch/x86/io.h>
#include <arch/x86/irq.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/apic.h>

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
    69,  /* Up Arrow */
    67,  /* Page Up */
   '-',
    70,  /* Left Arrow */
    0,
    71,  /* Right Arrow */
   '+',
    0,  /* 79 - End key*/
    72,  /* Down Arrow */
    68,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

unsigned char skbdlt[128] =
{
   0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b', /* BACKSPACE */
  '\t',     /* TAB */
  'Q', 'W', 'E', 'R', /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* ENTER KEY */
  0,      /* 29   - CONTROL */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
  '\"', '~',   0,    /* LEFT SHIFT */
  '|', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
   'M', '<', '>', '?',   0,        /* RIGHT SHIFT */
   '*',
    0,  /* ALT */
   ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    69,  /* Up Arrow */
    67,  /* Page Up */
   '-',
    70,  /* Left Arrow */
    0,
    71,  /* Right Arrow */
   '+',
    0,  /* 79 - End key*/
    72,  /* Down Arrow */
    68,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

u32 shift_make = 0;

internal void
keyboard_handler(const union biosregs *iregs)
{
  u8 scancode;

  scancode = inb(0x60);

  if (scancode & 0x80) //released
  {
    if (scancode == 0xAA)
    {
      shift_make = 0;
    }
  }
  else
  {
    if (scancode == 0x2A)
    {
      shift_make = 1;
    }

    switch(kbdlt[scancode])
    {
      case 67:
      {
        terminal_load_prev();
        break;
      }
      case 68:
      {
        terminal_load_next();
        break;
      }
      case 69:
        terminal_move(&state, TERM_DIRECTION_UP);
      {
        break;
      }
      case 70:
      {
        terminal_move(&state, TERM_DIRECTION_LEFT);
        break;
      }
      case 71:
      {
        terminal_move(&state, TERM_DIRECTION_RIGHT);
        break;
      }
      case 72:
      {
        terminal_move(&state, TERM_DIRECTION_DOWN);
        break;
      }
      case 0:
      {
        //ignore
        break;
      }
      default:
      {
        if (shift_make == 1)
        {
          terminal_put_char(&state, skbdlt[scancode]);
        }
        else
        {
          terminal_put_char(&state, kbdlt[scancode]);
        }
      }break;
    }
  }
}

void
keyboard_install(u16 refresh_rate)
{
  (void)(refresh_rate);
  irq_set_handler(1, keyboard_handler);
}