
#include "scatterlist.h"

#include <string.h>
#include <macros.h>

#include <arch/x86/framebuffer.h>

#define SLITER_BEGIN 0x1

struct scatterlist_iter
{
  struct scatterlist *list;
  u8 flags;

  void *buffer;
  u32 len;
};

void
slit_begin(struct scatterlist_iter *sliter, struct scatterlist *sl)
{
  sliter -> list = sl;
  sliter -> flags = SET_BIT(sliter -> flags, SLITER_BEGIN);
}

b8
slit_next(struct scatterlist_iter *sliter)
{
  struct scatterlist *list = sliter -> list;

  if (!IS_BIT_SET(sliter -> flags, SLITER_BEGIN) && SCATTERLIST_IS_LAST(list))
    return 0;

  sliter -> flags = CLEAR_BIT(sliter -> flags, SLITER_BEGIN);

  list++;
  if (SCATTERLIST_IS_CHAIN(list))
  {
    list = SCATTERLIST_NEXT_CHAIN(list);
  }

  sliter  -> list = list;
  sliter  -> buffer = (u8*)SCATTERLIST_PAGE_ADDR(list) + list -> off;
  sliter  -> len = list -> len;

  return 1;
}

b8
slit_skip(struct scatterlist_iter *sliter, u32 skip)
{
  while (skip)
  {
    if (!slit_next(sliter))
      return 0;

    u32 consume = min(sliter -> len, skip);
    sliter -> buffer = (u8*)sliter -> buffer + consume;
    sliter -> len -= consume;
  }

  return 1;
}

u32
sl_copy_buffer(struct scatterlist *sl, u32 skip, u8 *buffer, u32 buf_len, u8 direction)
{
  u32 offset = 0;
  struct scatterlist_iter sliter;
  slit_begin(&sliter, sl);

  if (!slit_skip(&sliter, skip))
    return 0;

  while (buf_len && slit_next(&sliter))
  {
    u32 len = min(sliter.len, buf_len);
    if (direction == SL_IN)
      memcpy(sliter.buffer,  buffer, len);
    else if (direction == SL_OUT)
      memcpy(buffer, sliter.buffer, len);

    offset += len;
    buf_len -= len;
  }

  return offset;
}

#define SCATTERLIST_SET_CHAIN(s) ((s) -> page_addr = SET_BIT((s) -> page_addr, 0x2))
#define SCATTERLIST_SET_ADDR(s, addr)  ((s) -> page_addr = ((s) -> page_addr & 0x3) | (u32)(addr))
#define SCATTERLIST_SET_LAST(s) ((s) -> page_addr = SET_BIT((s) -> page_addr, 0x1))

b8
sl_chain(struct scatterlist *sl1, struct scatterlist *sl2)
{
  struct scatterlist_iter sliter;
  slit_begin(&sliter, sl1);

  if (slit_next(&sliter))
  {
    struct scatterlist *chain = sliter.list;
    SCATTERLIST_SET_CHAIN(chain);
    SCATTERLIST_SET_ADDR(chain, sl2);
    return 1;
  }

  return 0;
}

void
sl_list_init(struct scatterlist *list, u32 nents)
{
  struct scatterlist *lastent = &list[nents - 1];
  SCATTERLIST_SET_LAST(lastent);
}

void
sl_bind_buffer(struct scatterlist *sl, void *buffer, u32 len)
{
  u32 page_addr = (u32)buffer & (~(kb(4) - 1));
  u32 offset = (u32)buffer - page_addr;

  SCATTERLIST_SET_ADDR(sl, page_addr);
  sl -> off = offset;
  sl -> len = len;
}

void*
sl_get_buffer(struct scatterlist *sl)
{
  return (void*)(SCATTERLIST_PAGE_ADDR(sl) + sl -> off);
}