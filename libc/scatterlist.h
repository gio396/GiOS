#ifndef _SCATTERLIST_H_
#define _SCATTERLIST_H_

#include <common.h>
#include <assert.h>

#define SCATTERLIST_IS_LAST(s) ((s) -> page_addr & 0x2)
#define SCATTERLIST_IS_CHAIN(s) ((s) -> page_addr & 0x1)
#define SCATTERLIST_NEXT_CHAIN(s) ((struct scatterlist*)((s) -> page_addr & ~0x3))
#define SCATTERLIST_PAGE_ADDR(s) (u32)((s) -> page_addr & ~0x3)

#define SL_IN 0x0
#define SL_OUT 0x1

struct scatterlist
{
  u32 page_addr;
  u32 off;
  u32 len;
};

u32
sl_copy_buffer(struct scatterlist *sl, u32 skip, u8 *buffer, u32 buf_len, u8 direction);

void
sl_list_init(struct scatterlist *sl, u32 nents);

b8
sl_chain(struct scatterlist *sl0, struct scatterlist *sl1);

void
sl_bind_buffer(struct scatterlist *sl, void *buffer, u32  len);

void*
sl_get_buffer(struct scatterlist *sl);

#endif
