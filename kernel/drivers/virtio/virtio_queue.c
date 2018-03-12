#include "virtio_queue.h"
#include "virtio.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/page.h>

#include <memory.h>
#include <string.h>
#include <macros.h>

/* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT       1
/* This marks a buffer as device write-only (otherwise device read-only). */
#define VIRTQ_DESC_F_WRITE      2
/* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT   4
#define VIRTQ_USED_F_NO_NOTIFY  1

#define QALLIGN(x) (((x) + kb(4) - 1) & ~(kb(4) - 1))
static inline u32 virtq_size(u32 qsz)
{
return QALLIGN(sizeof(struct virtq_desc)*qsz + sizeof(u16)*(3 + qsz))
      + QALLIGN(sizeof(u16)*3 + sizeof(struct virtq_used_elem)*qsz);
}
#undef QALLIGN

struct virtio_queue*
virtio_create_queue(u32 len)
{
  static u32 offset = 0;
  struct virtio_queue *res = (struct virtio_queue*)kzmalloc(sizeof(struct virtio_queue));
  ZERO_STRUCT(res, struct virtio_queue);

  res -> num_added = 0;
  res -> size      = len;

  u32 sz = virtq_size(len);

  void *buffer = kalloc((sz + kb(4) - 1) / kb(4));
  memset(buffer, 0, sz);
  offset += sz;

  res -> desc = (struct virtq_desc*)(buffer);
  res -> avail = (struct virtq_avail*)(buffer + sizeof(struct virtq_desc) * len);
  res -> used = (struct virtq_used*) (((u32) &res -> avail -> ring[len] + sizeof(u16) + 4096-1) & ~(4096-1));

  assert1(ALIGNED(res -> desc, 16));
  assert1(ALIGNED(res -> avail, 2));
  assert1(ALIGNED(res -> used,  4));

  res -> next_buffer = 0;
  res -> avail -> flags = 0;

  //TODO(gio): chain all the values 
  for (u32 i = 0; i < len; i++)
  {
    res -> desc[i].next = i + 1;
  }
  res -> desc[len - 1].next = 0;

  return res;
}

void
virtio_queue_enqueue(struct virtio_queue *q, u8 *buffer, size_t len, u8 direction)
{
  u16 index = q -> avail -> idx  % q -> size;
  u16 buffer_index = q -> next_buffer;
  u16 next_buffer_index = (buffer_index + 1) % q->size;

  q -> avail -> ring[index] = buffer_index;
  q -> desc[buffer_index].flags = direction ? VIRTQ_DESC_F_WRITE : 0;
  q -> desc[buffer_index].next = next_buffer_index;
  q -> desc[buffer_index].len = len;
  q -> desc[buffer_index].addr = (size_t)(buffer);
  buffer_index = next_buffer_index;

  q -> next_buffer = buffer_index;

  // NOTE(gio): virtqs are only unidirectional.
  // if (!direction)
  // {
  //   q -> last_buffer_seen = buffer_index;
  // }

  q -> num_added++;
}

struct scatterlist
virtio_queue_dequeue(struct virtio_queue *q)
{
  struct scatterlist res;
  sl_list_init(&res, 1);
  u16 buffer_index = q -> last_buffer_seen % q -> size;

  struct virtq_used_elem *elem = &q -> used -> ring[buffer_index];
  u32 len = elem -> len;
  u8  *buffer = (u8*)(size_t)q -> desc[buffer_index].addr;

  sl_bind_buffer(&res, buffer, len);
  q -> last_buffer_seen++;

  return res;
}

void
virtio_queue_kick(struct virtio_queue *q)
{
  q -> avail -> idx = q -> avail -> idx + q -> num_added;
  q -> num_added = 0;

  if (!(q -> used -> flags & VIRTQ_USED_F_NO_NOTIFY))
  {
    virtio_queue_notify(q);
  }
}

void
virtio_queue_notify(struct virtio_queue *q)
{
  u32 iobase = q -> vdev -> iobase;
  u32 idx = q -> idx;
  asm volatile("mfence" ::: "memory");
  virtio_header_set_word(iobase, OFFSET_OF(struct virtio_header, queue_notify), (u8*)&idx); 
}

u16
virtio_queue_num_heads(struct virtio_queue *vq)
{
  if (vq -> last_buffer_seen < vq -> used -> idx)
  {
    return vq -> used -> idx - vq -> last_buffer_seen;
  }

  return 0;
}

i8
virtio_queue_has_unseen_buffers(struct virtio_queue *q)
{
  return (virtio_queue_num_heads(q) > 0);
}