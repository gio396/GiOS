#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <common.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_SIZE   VGA_WIDTH * VGA_HEIGHT * 16

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

typedef struct 
{
  int32 terminal_row;
  int32 terminal_column;
  uint8 terminal_color;
  uint16* terminal_buffer;
} terminal_state;

void terminal_put_char(terminal_state *state, const int8 c);
void terminal_init(terminal_state *state);
void terminal_put_string(terminal_state *state, const int8 *s);

int32
write(uint8 * buffer, uint8 *data, int32 size);

void
printk(terminal_state *state, const int8 *format, ...);

extern terminal_state state;

#endif
