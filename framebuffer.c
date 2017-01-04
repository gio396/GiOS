#include "framebuffer.h"

#include "io.h"
#include "string.h"

terminal_state state;

#define VGA_COLOR(bg, fg) \
  ((bg) << 4) | (fg)

#define VGA_CHAR_COLOR(uc, col)\
  (uint16)(uc) | (uint16)(col) << 8

//TODO: move this to seperate place
int32
write(uint8 *buffer, uint8 *data, int32 size)
{
  int32 result = 0;

  while (result < size)
  {
    buffer[result] = data[result];
    result++;
  }

  return result;
}

#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

internal void 
terminal_move_cursor(uint32 index)
{
  outb (FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
  outb(FB_DATA_PROT, ((index >> 8) & 0x00FF));
  outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
  outb(FB_DATA_PROT, (index & 0x00FF));
}


internal uint32 
terminal_advance_one(terminal_state *state)
{
  uint32 result;

  result = state->terminal_column + state->terminal_row * VGA_WIDTH;

  if (++state->terminal_column > VGA_WIDTH)
  {
    state->terminal_column = 0;

    if (++state->terminal_row > VGA_HEIGHT)
    {
      state->terminal_row = 0;
    }
  }


  terminal_move_cursor(state->terminal_column + state->terminal_row * VGA_WIDTH);
  return result;
}

internal void 
terminal_next_line(terminal_state *state)
{
  state->terminal_column = 0;

  if (state->terminal_row++ > VGA_HEIGHT)
  {
    state->terminal_row = 0;
  }

  terminal_move_cursor(state->terminal_column + state->terminal_row * VGA_WIDTH);
}

void 
terminal_put_char(terminal_state *state, const int8 c)
{
  int32 index = terminal_advance_one(state);
  int16 data = VGA_CHAR_COLOR(c, state->terminal_color);

  write((uint8*)(state->terminal_buffer + index), (uint8*)&data, 2);
}

void 
terminal_put_string(terminal_state *state, const int8 *s)
{
  int8 *it = (int8*)s;
  int8 c;

  while ((c = *it++))
  {
    if(c == '\n')
    {
      terminal_next_line(state);
    }
    else
    {
      terminal_put_char(state, c);
    }
  }
}

void 
terminal_init(terminal_state *state)
{
  state->terminal_row = 0;
  state->terminal_column = 0;
  state->terminal_buffer = (uint16 *)(0xb8000);
  state->terminal_color = VGA_COLOR(COLOR_BLACK, COLOR_LIGHT_GREY);

  terminal_move_cursor(0);

  uint16 def_color = VGA_CHAR_COLOR('\0', state -> terminal_color);

  for (int32 i = 0; i < VGA_HEIGHT; i++)
  {
    for (int32 j = 0; j < VGA_WIDTH; j++)
    {
      const int32 index = i * VGA_WIDTH + j;

      write((uint8 *)(state->terminal_buffer + index), (uint8 *)&def_color, 2);
    }
  }
}

#define MAX_PRINTK 256

#ifndef stdarg_hxx
#define stdarg_hxx

typedef void * va_list;

#define __va_size( type ) \
( ( sizeof( type ) + 3 ) & ~0x3 )

#define va_start( va_l, last ) \
( ( va_l ) = ( void * )&( last ) + __va_size( last ) )

#define va_end( va_l )

#define va_arg( va_l, type ) \
( ( va_l ) += __va_size( type ), \
*( ( type * )( ( va_l ) - __va_size( type ) ) ) )

#endif

void
printk(terminal_state *state, const int8 *format, ...)
{
  va_list args;
  va_start(args, format);

  while(*format != '\0')
  {
    if (*format == '%')
    {
      switch (*++format)
      {
        case 'd':
        {
          int32 val = va_arg(args, int32);

          char buffer[24];
          itoa(val, buffer);
          terminal_put_string(state, buffer);
        }
        break;
        case 'c':
        {
          int8 val = va_arg(args, int8);
          terminal_put_char(state, val);
        }
        break;
        case 's':
        {
          int8 *val = (int8*)va_arg(args, int32);
          terminal_put_string(state, val);
        }
      }
    }
    else if(*format == '\\')
    {
      terminal_put_char(state, *format);
    }
    else if(*format == '\n')
    {
      terminal_next_line(state);
    }
    else
    {
      terminal_put_char(state, *format);
    }

    format++;
  }
}