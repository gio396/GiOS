#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <common.h>
#include <list.h>

#define COLOR_BLACK          (int8)(0)
#define COLOR_BLUE           (int8)(1)
#define COLOR_GREEN          (int8)(2)
#define COLOR_CYAN           (int8)(3)
#define COLOR_RED            (int8)(4)
#define COLOR_MAGENTA        (int8)(5)
#define COLOR_BROWN          (int8)(6)
#define COLOR_LIGHT_GREY     (int8)(7)
#define COLOR_DARK_GREY      (int8)(8)
#define COLOR_LIGHT_BLUE     (int8)(9)
#define COLOR_LIGHT_GREEN    (int8)(10)
#define COLOR_LIGHT_CYAN     (int8)(11)
#define COLOR_LIGHT_RED      (int8)(12)
#define COLOR_LIGHT_MAGENTA  (int8)(13)
#define COLOR_LIGHT_BROWN    (int8)(14)
#define COLOR_LIGHT_WHITE    (int8)(15)

#define VGA_WIDTH        80
#define VGA_HEIGHT       25
#define VGA_LENGTH       VGA_WIDTH * VGA_HEIGHT 

#define SEG_WRITE(x)     (x) << 0
#define SEG_STLIMIT(x)   (x) << 1
#define SEG_LIMIT(x)     (x) << 2

#define SEG_WRITE_ENABLED 0x01
#define SEG_LIMIT_ENABLED 0x02
#define SEG_GET_LIMIT     0xFC

#define DEFAULT_TERMINAL   SEG_WRITE(1) | SEG_STLIMIT(0) | SEG_LIMIT(0)
#define SECONDARY_TERMINAL SEG_WRITE(0) | SEG_STLIMIT(1) | SEG_LIMIT(20)

#define TERM_DIRECTION_UP     0
#define TERM_DIRECTION_DOWN   1
#define TERM_DIRECTION_LEFT   2
#define TERM_DIRECTION_RIGHT  3

//terminal flags
//8         2    1   0
//[LMT------|STL-|wr-]

struct terminal_back_list
{
  uint16 left;
  uint16 buffer[VGA_LENGTH];

  struct dlist_node node;
};

struct terminal_state 
{
  int32   terminal_row;
  int32   terminal_column;
  uint8   terminal_color;
  uint8   terminal_flags;
  uint16* terminal_buffer;

  uint8   terminal_current_length; 
  struct dlist_root cur;
  struct dlist_root head;
};

void 
terminal_put_char(struct terminal_state *state, const int8 c);

void 
terminal_init(struct terminal_state *state);

void 
terminal_put_string(struct terminal_state *state, const int8 *s);

void
terminal_load_prev();

void
terminal_load_next();

int32
terminal_move(struct terminal_state *state, int32 direction);

void
printk(struct terminal_state *state, const int8 *format, ...);

extern struct terminal_state state;

#endif
