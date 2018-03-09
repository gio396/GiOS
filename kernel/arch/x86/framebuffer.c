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
  ((i16)(uc) | ((i16)(col) << 8))

#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

#define VGA_SIZE         VGA_WIDTH * VGA_HEIGHT * sizeof(i16)
#define VGA_PSIZE        sizeof(i16)
#define VGA_MEM_LOCATION 0xC00B8000

#define CURSOR_CHAR_CODE   219
#define CURSOR_CHAR(state) VGA_CHAR_COLOR(CURSOR_CHAR_CODE, (state).terminal_color)

i16 *vga_buffer = (i16*)(VGA_MEM_LOCATION); 
struct terminal_state state;
struct terminal_state *current_state;

i32 cursor_char = 0;

internal void 
vga_move_cursor(u32 index)
{
  outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
  outb(FB_DATA_PROT, ((index >> 8) & 0x00FF));
  outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
  outb(FB_DATA_PROT, (index & 0x00FF));
}

internal void
vga_set_char(u32 index, i16 val)
{
  vga_buffer[index] = val;
}

internal i16
vga_get_char(u32 index)
{
  return vga_buffer[index];
}

internal void
terminal_set_char(struct terminal_state* state, u32 index, i16 val)
{
  state->terminal_buffer[index] = val;
}

internal void
terminal_clear_row(struct terminal_state *state)
{
  u32 offset = state->terminal_row * VGA_WIDTH;
  i16 val = VGA_CHAR_COLOR(0, state->terminal_color);

  for (int i = 0; i < VGA_WIDTH; i++)
    terminal_set_char(state, offset + i, val);
}

internal void
terminal_copy_buffer(struct terminal_state *state, 
                     i16 index, 
                     i16 *dst, 
                     i16 size)
{
  memcpy(state->terminal_buffer + index, dst, size * VGA_PSIZE);
}

i32
terminal_move_(struct terminal_state *state, i32 direction)
{
  i32 last_row = 0;
  i32 last_column = 0;
  i32 res = 0;

  if (state == current_state)
  {
    last_row = state -> terminal_row;
    last_column = state -> terminal_column;
  }

  switch (direction)
  {
    case TERM_DIRECTION_UP:
    {
      --state -> terminal_row;

      if (state -> terminal_row < 0)
      {
        state -> terminal_row = VGA_HEIGHT - 1;
        res = 1;
      }

      break;
    }
    case TERM_DIRECTION_DOWN:
    {
      ++state -> terminal_row;

      if (state -> terminal_row >= VGA_HEIGHT)
      {
        state -> terminal_row = 0;
        res = 1;
      }
      break;
    }
    case TERM_DIRECTION_LEFT:
    {
      --state -> terminal_column;

      if (state -> terminal_column < 0)
      {
        state -> terminal_column = VGA_WIDTH - 1;
        --state -> terminal_row;
        res = 1;

        if (state -> terminal_row < 0)
        {
          state -> terminal_row = VGA_HEIGHT - 1;
          res = 2;
        }
      }
      break;
    }
    case TERM_DIRECTION_RIGHT:
    {
      ++state -> terminal_column;

      if (state -> terminal_column >= VGA_WIDTH)
      {
        state -> terminal_column = 0;
        ++state -> terminal_row;
        res = 1;
        if (state -> terminal_row >= VGA_HEIGHT)
        {
          state -> terminal_row = 0;
          res = 2;
        }
      }
      break;
    }
  }

  if (state == current_state && state -> terminal_buffer == vga_buffer)
  {
    vga_set_char(last_row * VGA_WIDTH + last_column, cursor_char);
    u32 index = state -> terminal_row * VGA_WIDTH + state -> terminal_column;

    cursor_char = vga_get_char(index);
    vga_set_char(index, CURSOR_CHAR(*state));
  }

  return res;
}

internal void
terminal_save_state(struct terminal_state *state)
{
  struct dlist_node *new_head, *head;
  struct dlist_root *head_root;
  struct terminal_back_list *tbl_head, *tbl_new_head;
  i16 default_empty_char;
  i32 buffer_offset, right_edge, left_edge, length;


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
    tbl_new_head = (struct terminal_back_list*)kalloc(1); 
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

  for(i32 i = 0; i < VGA_LENGTH; i++)
    terminal_set_char(state, i, default_empty_char);

  head_root->dlist_node = new_head;
}

void
terminal_load_head()
{
  i16 default_empty_char, used;
  i32 idx;
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
  i16 used;
  i32 idx;

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
  i16 default_empty_char, used; 
  i32 idx;


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

internal u32 
terminal_advance_one(struct terminal_state *state)
{
  struct dlist_root head_root;
  struct terminal_back_list *tbl_head;
  u32 result;

  result = state->terminal_column + state->terminal_row * VGA_WIDTH;

  if (state->terminal_buffer != vga_buffer)
  {
    head_root = state->head;
    tbl_head = CONTAINER_OF(head_root.dlist_node, struct terminal_back_list, node); 
    tbl_head->left--;
  }

  i32 op = terminal_move_(state, TERM_DIRECTION_RIGHT);

  switch (op)
  {
    case 2:
      terminal_save_state(state);
    case 1:
      terminal_clear_row(state);
  }

  return result;
}

internal u32
terminal_delete_one(struct terminal_state *state)
{
  struct dlist_root head_root;
  struct terminal_back_list *tbl_head;
  u32 result;
  i16 default_empty_char;


  default_empty_char = VGA_CHAR_COLOR(0, state->terminal_color);

  if (state->terminal_buffer != vga_buffer)
  {
    head_root = state->head;
    tbl_head = CONTAINER_OF(head_root.dlist_node, struct terminal_back_list, node);
    tbl_head->left++;
  }

  result = state->terminal_column + state->terminal_row * VGA_WIDTH - 1;

  terminal_set_char(state, result, default_empty_char);

  terminal_move_(state, TERM_DIRECTION_LEFT);

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
  else
  {
    vga_set_char(state -> terminal_row * VGA_WIDTH + state -> terminal_column, cursor_char);
  }

  state -> terminal_column = 0;

  if (++state->terminal_row >= VGA_HEIGHT)
  {
    terminal_save_state(state);
    state->terminal_row = 0;
  }

  terminal_clear_row(state);
  terminal_put_string(state, ">>");


  u32 index = state -> terminal_row * VGA_WIDTH + state -> terminal_column;
  cursor_char = VGA_CHAR_COLOR('\0', state -> terminal_color);
  vga_set_char(index, VGA_CHAR_COLOR(CURSOR_CHAR(*state), state -> terminal_color));
}

void 
terminal_put_char(struct terminal_state *st, i8 c)
{
  i32 idx;
  i16 val;

  if(st == current_state && current_state->terminal_buffer != vga_buffer)
    terminal_load_head();

  #ifdef __GIOS_DEBUG__
  if(st == &state && st->output_to_serial != 0)
  {
    write_serial(c);
  }
  #endif

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
terminal_put_string(struct terminal_state *state, const i8 *s)
{
  i8 *it = (i8*)s;
  i8 c;

  while (*it)
  {
    c = *(it++);
    
    terminal_put_char(state, c);
  }
}

void 
terminal_init(struct terminal_state *state)
{
  i32 idx;
  i16 default_empty_char;

  default_empty_char = VGA_CHAR_COLOR(0, state -> terminal_color);
  state->terminal_row = 0;
  state->terminal_column = 0;
  state->terminal_buffer = (i16 *)(VGA_MEM_LOCATION);
  state->terminal_color = VGA_COLOR(COLOR_BLACK, COLOR_GREEN);
  state->output_to_serial = 0;

  state->head.dlist_node = NULL;
  state->cur.dlist_node = NULL;

  current_state = state;

  vga_move_cursor(-1); // disable cursor we will handle it manually;;

  for (idx = 0; idx < VGA_LENGTH; idx++)
      terminal_set_char(state, idx, default_empty_char);
}

i32
get_width(const i8 *str)
{
  i32 res = 0;
  i8 *c = (i8*)str;

  while(*c >= '0' && *c <= '9')
  {
    res *= 10;
    res += (*c - '0');
    ++c;
  }

  return res;
}

void
printk(struct terminal_state *state, const i8 *format, ...)
{
  i8 nxt, chr, *str;
  const i8 *run_start;
  b32 run;
  i32 width, val, len;
  i8 wchar = 0;

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

      width = get_width(run_start);

      if (width > 0)
      {
        wchar = (*run_start == '0' ? '0':' ');
      }

      switch (nxt)
      {
        case 'd':
        case 'i':
        {
          val = va_arg(args, i32);
          i8 buffer[12];

          itoa(val, buffer, 10);

          if (run)
          {
            len = strlen(buffer);
            while (len++ < width)
              terminal_put_char(state, wchar);
          }

          terminal_put_string(state, buffer);
          break;
        }

        case 'u':
        {
          val = va_arg(args, i32);
          i8 buffer[12];
          uitoa(val, buffer, 10);

          if (run)
          {
            len = strlen(buffer);
            while (len++ < width)
              terminal_put_char(state, wchar);
          }

          terminal_put_string(state, buffer);
          break;
        }

        case 'x':
        {
          val = va_arg(args, i32);
          i8 buffer[12];
          itoa(val, buffer, 16);

          if (run)
          {
            len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, wchar);

          }

          terminal_put_string(state, buffer);
          break;
        }


        case 'X':
        {
          val = va_arg(args, i32);
          i8 buffer[12];
          itoa(val, buffer, 16);

          if (run)
          {
            len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, wchar);

          }

          to_upper(buffer);
          terminal_put_string(state, buffer);
          break;
        }

        case 'P':
        case 'p':
        {
          val = va_arg(args, size_t);
          i8 buffer[12];
          itoa(val, buffer, 16);

          terminal_put_string(state, "0X");
          if (run)
          {
            len = strlen(buffer);
            while (len++ < width)
              terminal_put_char(state, wchar);
          }

          if (nxt == 'P')
            to_upper(buffer);
          
          terminal_put_string(state, buffer);
          break;
        }

        case 'b':
        {
          val = va_arg(args, i32);
          i8 buffer[33];
          itoa(val, buffer, 2);

          if (run)
          {
            len = strlen(buffer);
            while(len++ < width)
              terminal_put_char(state, wchar);

          }

          terminal_put_string(state, buffer);
          break;
        }

        case 'c':
        {
          chr = va_arg(args, i32);
          terminal_put_char(state, chr);
          break;
        }

        case 's':
        {
          str = va_arg(args, i8*);
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
    else
    {
      terminal_put_char(state, *format);
    }

    format++;
  }
}

void
terminal_move(struct terminal_state *state, i32 direction)
{
  terminal_move_(state, direction);
}
