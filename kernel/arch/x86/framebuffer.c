#include "framebuffer.h"

#include <arch/x86/io.h>
#include <arch/x86/page.h>

#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <macros.h>
#include <list.h>
#include <assert.h>

#define VGA_COLOR(bg, fg) \
  ((bg) << 4) | (fg)

#define VGA_CHAR_COLOR(uc, col)\
  ((uint16)(uc) | ((uint16)(col) << 8))

#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

#define VGA_SIZE         VGA_WIDTH * VGA_HEIGHT * sizeof(uint16)
#define VGA_PSIZE        sizeof(uint16)
#define VGA_MEM_LOCATION 0xC00B8000

uint16 *vga_buffer = (uint16*)(VGA_MEM_LOCATION); 
struct terminal_state state;
struct terminal_state *current_state;

internal void 
vga_move_cursor(uint32 index)
{
  outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
  outb(FB_DATA_PROT, ((index >> 8) & 0x00FF));
  outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
  outb(FB_DATA_PROT, (index & 0x00FF));
}

internal void
vga_set_char(uint32 index, uint16 val)
{
  vga_buffer[index] = val;
}

internal void
terminal_set_char(struct terminal_state* state, uint32 index, uint16 val)
{
  state->terminal_buffer[index] = val;
}

internal void
terminal_clear_row(struct terminal_state *state)
{
  uint32 offset = state->terminal_row * VGA_WIDTH;
  uint16 val = VGA_CHAR_COLOR(0, state->terminal_color);

  for (int i = 0; i < VGA_WIDTH; i++)
    terminal_set_char(state, offset + i, val);
}

internal void
terminal_copy_buffer(struct terminal_state *state, 
                     uint16 index, 
                     uint16 *dst, 
                     uint16 size)
{
  memcpy(state->terminal_buffer + index, dst, size * VGA_PSIZE);
}

internal void
terminal_save_state(struct terminal_state *state)
{
  struct terminal_back_list *new_head = NULL;
  struct terminal_back_list *head = (struct terminal_back_list*)state->head;
  uint16 default_empty_char =  VGA_CHAR_COLOR(0, state->terminal_color);

  if (head && head->left > 0)
  {
    new_head = head;
  }
  else if ((state->terminal_flags & SEG_LIMIT_ENABLED) && 
          ((state->terminal_flags & SEG_GET_LIMIT) == 
            state->terminal_current_length))
  {
    new_head = DLIST_GET_TAIL(state->head, struct terminal_back_list);
    if(new_head->next)
      new_head->next->prev = NULL;

    DLIST_INSERT_HEAD(head, new_head);

    new_head->left = VGA_LENGTH;
    state->head = new_head;
  }
  else
  {
    new_head = (struct terminal_back_list*)kalloc();

    DLIST_INSERT_HEAD(head, new_head);

    state->head = new_head;
    new_head->left = VGA_LENGTH;
  }

  if(state->terminal_buffer == vga_buffer)
  { 
    int32 buffer_offset = VGA_LENGTH - new_head->left;
    int32 right_edge = state->terminal_row * (VGA_WIDTH) + state->terminal_column;
    int32 left_edge = VGA_LENGTH - new_head->left;
    int32 length = right_edge - left_edge;

    if(length > 0)
    {
      terminal_copy_buffer(state, 
                           buffer_offset, 
                           new_head->buffer + buffer_offset,
                           length);

      new_head->left -= length;
    }
  }
  else
  {
    state->terminal_buffer = new_head->buffer;
  }

  for(int32 i = 0; i < VGA_LENGTH; i++)
    terminal_set_char(state, i, default_empty_char);

  state->head = (void*)new_head;
}

void
terminal_load_head()
{
  if (current_state->terminal_buffer != vga_buffer)
  {
    uint16 default_empty_char = VGA_CHAR_COLOR(0, current_state->terminal_color); 
    struct terminal_back_list *next = current_state->head;
    uint16 used = VGA_LENGTH - next->left;

    for (int32 i = 0; i < used; i++)
      vga_set_char(i, next->buffer[i]);

    for (int32 i = used; i < VGA_LENGTH; i++)
      vga_set_char(i, default_empty_char);

    current_state->cur = next;
    current_state->terminal_buffer = vga_buffer;
  }
}

void
terminal_load_prev()
{
  struct terminal_back_list *cur = current_state->cur;
  struct terminal_back_list *head = current_state->head;
  struct terminal_back_list *prev;

  if (cur)
  {
    prev = (void*)cur->prev; 
  }
  else
  {
    prev = head;
  }

  if (prev)
  {
    if ((prev == head) || 
        ((cur == head) && (current_state->terminal_buffer == vga_buffer)))
    {
      terminal_save_state(current_state);
      current_state->terminal_buffer = current_state->head->buffer;
    }

    uint16 used = VGA_LENGTH - prev->left;

    for (int32 i = 0; i < used; i++)
      vga_set_char(i, prev->buffer[i]);

    current_state->cur = prev;
  }
}

void
terminal_load_next()
{
  struct terminal_back_list *cur = current_state->cur;
  struct terminal_back_list *next = NULL;
  uint16 default_empty_char = VGA_CHAR_COLOR(0, current_state->terminal_color); 

  if (cur)
    next = cur->next;

  if (next)
  {
    uint16 used = VGA_LENGTH - next->left;

    for (int32 i = 0; i < used; i++)
      vga_set_char(i, next->buffer[i]);

    for (int32 i = used; i < VGA_LENGTH; i++)
      vga_set_char(i, default_empty_char);

    current_state->cur = next;

    if(next == current_state->head)
      current_state->terminal_buffer = vga_buffer;
  }
}

internal uint32 
terminal_advance_one(struct terminal_state *state)
{
  uint32 result;

  result = state->terminal_column + state->terminal_row * VGA_WIDTH;

  if (state->terminal_buffer != vga_buffer)
  {
    state->head->left--;
  }

  if (++state->terminal_column >= VGA_WIDTH)
  {
    state->terminal_column = 0;

    if (++state->terminal_row >= VGA_HEIGHT)
    {
      terminal_save_state(state);

      state->terminal_row = 0;
    }

    terminal_clear_row(state);
  }

  return result;
}

internal uint32
terminal_delete_one(struct terminal_state *state)
{
  uint32 result;
  uint16 data = VGA_CHAR_COLOR(0, state->terminal_color);

  if (state->terminal_buffer != vga_buffer)
  {
    state->head->left++;
  }

  result = state->terminal_column + state->terminal_row * VGA_WIDTH - 1;

  terminal_set_char(state, result, data);

  if (--state->terminal_column <= 0)
  {
    state->terminal_column = VGA_WIDTH - 1;

    if (--state->terminal_row <= 0)
    {
      state->terminal_row = VGA_HEIGHT - 1;
    }
  }

  return result;
}


internal void 
terminal_next_line(struct terminal_state *state)
{
  if (state->terminal_buffer != vga_buffer)
  {
    state->head->left -= VGA_WIDTH - state->terminal_column;
  }

  state->terminal_column = 0;

  if (++state->terminal_row >= VGA_HEIGHT)
  {
    terminal_save_state(state);
    state->terminal_row = 0;
  }

  terminal_clear_row(state);
  terminal_put_string(state, ">> ");
}

void 
terminal_put_char(struct terminal_state *st, int8 c)
{
  if(st == current_state && current_state->terminal_buffer != vga_buffer)
    terminal_load_head();

  // #ifdef QEMU_DBG
  if(st == &state)
  {
    write_serial(c);
  }
  // #endif

  switch(c)
  {
    CASE(
      terminal_delete_one(st);
    , '\b');

    CASE(
      terminal_next_line(st);
    , '\n');

    default:
    {
      int32 index = terminal_advance_one(st);
      int16 val = VGA_CHAR_COLOR(c, st->terminal_color);

      terminal_set_char(st, index, val);
    }
  }
}

void 
terminal_put_string(struct terminal_state *state, const int8 *s)
{
  int8 *it = (int8*)s;
  int8 c;

  while (*it)
  {
    c = *(it++);
    
    terminal_put_char(state, c);
  }
}

void 
terminal_init(struct terminal_state *state)
{
  state->terminal_row = 0;
  state->terminal_column = 0;
  state->terminal_buffer = (uint16 *)(VGA_MEM_LOCATION);
  state->terminal_color = VGA_COLOR(COLOR_BLACK, COLOR_GREEN);

  current_state = state;

  vga_move_cursor(-1); // disable cursor we will handle it manually;;

  uint16 def_color = VGA_CHAR_COLOR('\0', state -> terminal_color);

  for (int32 i = 0; i < VGA_HEIGHT; i++)
  {
    for (int32 j = 0; j < VGA_WIDTH; j++)
    {
      const int32 index = i * VGA_WIDTH + j;

      terminal_set_char(state, index, def_color);
    }
  }
}

//TODO(GIO): Make this handle unsigned and signed integers.
//         : Make this handle different formating options available in printf.
//         : Fix this mess.
void
printk(struct terminal_state *state, const int8 *format, ...)
{
  va_list args;
  va_start(args, format);

  while(*format != '\0')
  {
    if (*format == '%')
    {
      int8 nxt = *++format;
      const int8 *run_start = format;
      b32 run = 0;
      int32 width;

      while(nxt >= '0' && nxt <= '9')
      {
        nxt = *++format;
        run++;
      }

      width = atoi(run_start);

      switch (nxt)
      {
        //signed integer base 10
        //d
        CASE(
          int32 val = va_arg(args, int32);
          char buffer[12];
          itoa(val, buffer, 10);

          if (run)
          {
            int32 len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, '0');

          }

          terminal_put_string(state, buffer);
        , 'd', 'i');

        CASE(
          int32 val = va_arg(args, int32);
          char buffer[12];
          itoa(val, buffer, 16);

          if (run)
          {
            int32 len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, '0');

          }

          terminal_put_string(state, buffer);
        , 'x');

        CASE(
          int32 val = va_arg(args, int32);
          char buffer[12];
          itoa(val, buffer, 16);

          if (run)
          {
            int32 len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, '0');

          }

          to_upper(buffer);
          terminal_put_string(state, buffer);
        , 'X');

        CASE(
          int32 val = va_arg(args, int32);
          char buffer[33];
          itoa(val, buffer, 2);

          if (run)
          {
            int32 len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, '0');

          }

          terminal_put_string(state, buffer);
        , 'b');

        CASE(
          int8 val = va_arg(args, int8);
          terminal_put_char(state, val);
        , 'c');

        CASE(
          int8 *val = (int8*)va_arg(args, int32);
          terminal_put_string(state, val);
        , 's');

        CASE(
          terminal_put_char(state, '%');
        , '%');
      }
    }
    else if(*format == '\\')
    {
      terminal_put_char(state, *format);
    }
    else
    {
      terminal_put_char(state, *format);
    }

    format++;
  }
}