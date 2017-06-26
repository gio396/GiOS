#include "framebuffer.h"

#include <arch/x86/io.h>
#include <arch/x86/page.h>
#include <arch/x86/irq.h>

#include <string.h>
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
  struct dlist_node *new_head, *head;
  struct dlist_root *head_root;
  struct terminal_back_list *tbl_head, *tbl_new_head;
  uint16 default_empty_char;
  int32 buffer_offset, right_edge, left_edge, length;


  new_head = NULL;
  head_root = &state->head;
  head = head_root->dlist_node;
  tbl_head = CONTAINER_OF(head, struct terminal_back_list, node);
  tbl_new_head = NULL;

  default_empty_char = VGA_CHAR_COLOR(0, state->terminal_color);

  if (head && tbl_head->left > 0)
  {
    new_head = head;
  }
  else if ((state->terminal_flags & SEG_LIMIT_ENABLED) && 
          ((state->terminal_flags & SEG_GET_LIMIT) == 
            state->terminal_current_length))
  {
    new_head = dlist_get_tail(head);

    if(new_head->next)
      new_head->next->prev = NULL;

    dlist_insert_head(head_root, new_head);

    tbl_new_head = CONTAINER_OF(new_head, struct terminal_back_list, node);
    tbl_new_head->left = VGA_LENGTH;
  }
  else
  {
    tbl_new_head = (struct terminal_back_list*)kalloc(); 
    new_head = &tbl_new_head->node;

    dlist_insert_head(head_root, new_head);
    tbl_new_head->left = VGA_LENGTH;
  }

  if(state->terminal_buffer == vga_buffer)
  { 
    tbl_new_head = CONTAINER_OF(new_head, struct terminal_back_list, node);
    buffer_offset = VGA_LENGTH - tbl_new_head->left;
    right_edge = state->terminal_row * (VGA_WIDTH) + state->terminal_column;
    left_edge = VGA_LENGTH - tbl_new_head->left;
    length = right_edge - left_edge;

    if(length > 0)
    {
      terminal_copy_buffer(state, 
                           buffer_offset, 
                           tbl_new_head->buffer + buffer_offset,
                           length);

      tbl_new_head->left -= length;
    }
  }
  else
  {
    tbl_new_head = CONTAINER_OF(new_head, struct terminal_back_list, node);
    state->terminal_buffer = tbl_new_head->buffer;
  }

  for(int32 i = 0; i < VGA_LENGTH; i++)
    terminal_set_char(state, i, default_empty_char);

  head_root->dlist_node = new_head;
}

void
terminal_load_head()
{
  uint16 default_empty_char, used;
  int32 idx;
  struct dlist_root *cur_root;
  struct dlist_node *head;
  struct terminal_back_list *tbl_next;


  if (current_state->terminal_buffer != vga_buffer)
  {
    default_empty_char = VGA_CHAR_COLOR(0, current_state->terminal_color); 
    cur_root = &current_state->cur;
    head = current_state->head.dlist_node;
    tbl_next = CONTAINER_OF(head, struct terminal_back_list, node);
    used = VGA_LENGTH - tbl_next->left;

    for (idx = 0; idx < used; idx++)
      vga_set_char(idx, tbl_next->buffer[idx]);

    for (idx = used; idx < VGA_LENGTH; idx++)
      vga_set_char(idx, default_empty_char);

    cur_root->dlist_node = head;
    current_state->terminal_buffer = vga_buffer;
  }
}

void
terminal_load_prev()
{
  struct dlist_root *cur_root, *head_root;
  struct dlist_node *cur, *head, *prev;
  struct terminal_back_list *tbl_prev, *tbl_head;
  uint16 used;
  int32 idx;

  cur_root = &current_state->cur;
  cur = cur_root->dlist_node;
  head_root = &current_state->head;
  head = head_root->dlist_node;

  if (cur)
  {
    prev = cur->next;
  }
  else
  {
    prev = head;
  }

  if (prev)
  {
    tbl_prev = CONTAINER_OF(prev, struct terminal_back_list, node);

    if ((prev == head) || 
        ((cur == head) && (current_state->terminal_buffer == vga_buffer)))
    {
      tbl_head = CONTAINER_OF(head, struct terminal_back_list, node);
      terminal_save_state(current_state);
      current_state->terminal_buffer = tbl_head->buffer;
    }

    used = VGA_LENGTH - tbl_prev->left;

    for (idx = 0; idx < used; idx++)
      vga_set_char(idx, tbl_prev->buffer[idx]);

    cur_root->dlist_node = prev;
  }
}

void
terminal_load_next()
{
  struct dlist_root *cur_root;
  struct dlist_node *cur, *next;
  struct terminal_back_list *tbl_next;
  uint16 default_empty_char, used; 
  int32 idx;


  cur_root = &current_state->cur;
  cur = cur_root->dlist_node;
  next = NULL;

  if (cur)
    next = cur->prev;

  if (next)
  {
    default_empty_char = VGA_CHAR_COLOR(0, current_state->terminal_color);
    tbl_next = CONTAINER_OF(next, struct terminal_back_list, node); 
    used = VGA_LENGTH - tbl_next->left;

    for (idx= 0; idx < used; idx++)
      vga_set_char(idx, tbl_next->buffer[idx]);

    for (idx = used; idx < VGA_LENGTH; idx++)
      vga_set_char(idx, default_empty_char);

    cur_root->dlist_node = next;

    if(next == current_state->head.dlist_node)
      current_state->terminal_buffer = vga_buffer;
  }
}

internal uint32 
terminal_advance_one(struct terminal_state *state)
{
  struct dlist_root head_root;
  struct terminal_back_list *tbl_head;
  uint32 result;


  result = state->terminal_column + state->terminal_row * VGA_WIDTH;

  if (state->terminal_buffer != vga_buffer)
  {
    head_root = state->head;
    tbl_head = CONTAINER_OF(head_root.dlist_node, struct terminal_back_list, node); 
    tbl_head->left--;
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
  struct dlist_root head_root;
  struct terminal_back_list *tbl_head;
  uint32 result;
  uint16 default_empty_char;


  default_empty_char = VGA_CHAR_COLOR(0, state->terminal_color);

  if (state->terminal_buffer != vga_buffer)
  {
    head_root = state->head;
    tbl_head = CONTAINER_OF(head_root.dlist_node, struct terminal_back_list, node);
    tbl_head->left++;
  }

  result = state->terminal_column + state->terminal_row * VGA_WIDTH - 1;

  terminal_set_char(state, result, default_empty_char);

  if (--state->terminal_column <= 0)
  {
    state->terminal_column = VGA_WIDTH - 1;

    if (--state->terminal_row <= 0)
    {
      state->terminal_row = VGA_HEIGHT - 1;
    }
  }

  return(result);
}


internal void 
terminal_next_line(struct terminal_state *state)
{
  struct dlist_root head_root;
  struct terminal_back_list *tbl_head;


  if (state->terminal_buffer != vga_buffer)
  {
    head_root = state->head;
    tbl_head = CONTAINER_OF(head_root.dlist_node, struct terminal_back_list, node);

    tbl_head->left -= VGA_WIDTH - state->terminal_column;
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
  int32 idx;
  int16 val;

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
    case '\b':
    {
      terminal_delete_one(st);
      break;
    }

    case '\n':
    {      
      terminal_next_line(st);
      break;
    }

    default:
    {
      idx = terminal_advance_one(st);
      val = VGA_CHAR_COLOR(c, st->terminal_color);

      terminal_set_char(st, idx, val);
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
  int32 idx;
  uint16 default_empty_char;

  default_empty_char = VGA_CHAR_COLOR(0, state -> terminal_color);
  state->terminal_row = 0;
  state->terminal_column = 0;
  state->terminal_buffer = (uint16 *)(VGA_MEM_LOCATION);
  state->terminal_color = VGA_COLOR(COLOR_BLACK, COLOR_GREEN);

  state->head.dlist_node = NULL;
  state->cur.dlist_node = NULL;

  current_state = state;

  vga_move_cursor(-1); // disable cursor we will handle it manually;;

  for (idx = 0; idx < VGA_LENGTH; idx++)
      terminal_set_char(state, idx, default_empty_char);
}

void
printk(struct terminal_state *state, const int8 *format, ...)
{
  int8 nxt, chr, *str;
  const int8 *run_start;
  b32 run;
  int32 width, val, len;


  va_list args;
  va_start(args, format);

  while(*format != '\0')
  {
    if (*format == '%')
    {
      nxt = *++format;
      run_start = format;
      run = 0;

      while(nxt >= '0' && nxt <= '9')
      {
        nxt = *++format;
        run++;
      }

      width = atoi(run_start);

      switch (nxt)
      {
        //signed integer base 10
        case 'd':
        case 'i':
        {
          val = va_arg(args, int32);
          int8 buffer[12];

          itoa(val, buffer, 10);

          if (run)
          {
            len = strlen(buffer);
            while (len++ < width)
              terminal_put_char(state, '0');

          }

          terminal_put_string(state, buffer);
          break;
        }

        case 'u':
        {
          val = va_arg(args, int32);
          int8 buffer[12];
          uitoa(val, buffer, 10);

          if (run)
          {
            len = strlen(buffer);
            while (len++ < width)
              terminal_put_char(state, '0');
          }

          terminal_put_string(state, buffer);
          break;
        }

        case 'x':
        {
          val = va_arg(args, int32);
          int8 buffer[12];
          itoa(val, buffer, 16);

          if (run)
          {
            len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, '0');

          }

          terminal_put_string(state, buffer);
          break;
        }

        case 'X':
        {
          val = va_arg(args, int32);
          int8 buffer[12];
          itoa(val, buffer, 16);

          if (run)
          {
            len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, '0');

          }

          to_upper(buffer);
          terminal_put_string(state, buffer);
          break;
        }

        case 'b':
        {
          val = va_arg(args, int32);
          int8 buffer[33];
          itoa(val, buffer, 2);

          if (run)
          {
            len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, '0');

          }

          terminal_put_string(state, buffer);
          break;
        }

        case 'c':
        {
          chr = va_arg(args, int32);
          terminal_put_char(state, chr);
          break;
        }

        case 's':
        {
          str = va_arg(args, int8*);
          terminal_put_string(state, str);
          break;
        }

        case '%':
        {
          terminal_put_char(state, '%');
          break;
        }
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
