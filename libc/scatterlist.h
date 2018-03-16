#ifndef _SCATTERLIST_H_
#define _SCATTERLIST_H_

#include <common.h>
#include <assert.h>

#define SCATTERLIST_IS_CHAIN(s) ((s) -> page_addr & 0x1)
#define SCATTERLIST_IS_LAST(s) ((s) -> page_addr & 0x2)
#define SCATTERLIST_NEXT_CHAIN(s) ((struct scatterlist*)((s) -> page_addr & ~0x3))
#define SCATTERLIST_PAGE_ADDR(s) (u32)((s) -> page_addr & ~(kb(4) - 1))

#define SL_IN 0x0
#define SL_OUT 0x1

#define SL_LAST   (1 << 0)
#define SL_CHAIN  (1 << 1)

#define SL_USER_0 2
#define SL_USER_1 3
#define SL_USER_2 4

struct scatterlist
{
  u32 page_addr;
  u32 off;
  u32 len;
};

struct scatterlist_iter
{
  struct scatterlist *list;
  u8 flags;

  void *buffer;
  u32 len;
  u8  user_flags;
};

#define SLITER_GET_FLAG_VALUE(s, v) ((s) -> user_flags & (1 << ((v) - SL_USER_0))) == 1 << ((v) - SL_USER_0) 

u32
sl_copy_buffer(struct scatterlist *sl, u32 skip, u8 *buffer, u32 buf_len, u8 direction);

void
sl_list_init(struct scatterlist *sl, u32 nents);

b8
sl_chain(struct scatterlist *sl0, struct scatterlist *sl1);

void
sl_bind_buffer(struct scatterlist *sl, void *buffer, u32 len);

void
sl_bind_attribute(struct scatterlist *sl, u32 possition, u32 val);

void
sl_make_last(struct scatterlist *sl);

void*
sl_get_buffer(struct scatterlist *sl);

b8
slit_skip(struct scatterlist_iter *sliter, u32 skip);

b8
slit_next(struct scatterlist_iter *sliter);

void
slit_begin(struct scatterlist_iter *sliter, struct scatterlist *sl);

#endif
