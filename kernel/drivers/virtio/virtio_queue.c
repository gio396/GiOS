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

struct virtio_queue*
virtio_create_queue(i8 *name, u32 len)
{
  static u32 offset = 0;
  struct virtio_queue *res = (struct virtio_queue*)kzmalloc(sizeof(struct virtio_queue));
  LOG("start:%p end:%d\n", res, sizeof(struct virtio_queue));

  res -> name = name;
  res -> free_head = 0;
  res -> num_added = 0;
  res -> size      = len;

  u32 sz = virtq_size(len);

  void* buffer = (void*)(0x0e000000 + offset);
  mmap((void*)(0x0e000000 + offset), sz, 0);
  offset += sz;
  memset(buffer, 0, sz);

  res -> desc = (struct virtq_desc*)(buffer);
  res -> avail = (struct virtq_avail*)(buffer + sizeof(struct virtq_desc) * len);
  res -> used = (struct virtq_used*) (((u32) &res -> avail -> ring[len] + sizeof(u16) + 4096-1) & ~(4096-1));

  assert1(ALIGNED(res -> desc, 16));
  assert1(ALIGNED(res -> avail, 2));
  assert1(ALIGNED(res -> used,  4));

  //TODO(gio:) chain all the values 
  for (u32 i = 0; i < len; i++)
  {
    res -> desc[i].next = i + 1;
  }
  res -> desc[len - 1].next = 0;

  LOGV("%d", res -> size);

  return res;
}

void
virtio_queue_enqueue(struct virtio_queue* q, u8 *buffer, size_t len)
{
  u16 head = q -> free_head;

  q -> desc[head].flags = VIRTQ_DESC_F_WRITE;

  q -> desc[head].addr = (u64)((u32)buffer);

  q -> desc[head].len  = (u32)len ;

  q -> free_head = q -> desc[head].next;

  u16 avail_idx = (q -> avail -> idx + q -> num_added) % q -> size;
  q -> num_added++;

  q -> avail -> ring[avail_idx] = head;
}

void
virtio_queue_kick(struct virtio_queue *q, u16 iobase)
{
  q -> avail -> idx = q -> avail -> idx + q -> num_added;
  q -> num_added = 0;
  u32 idx = q -> idx;

  if (!(q -> used -> flags & VIRTQ_USED_F_NO_NOTIFY))
  {
    asm volatile("mfence" ::: "memory");
    virtio_header_set_word(iobase, OFFSET_OF(struct virtio_header, queue_notify), (u8*)&idx);
  }
}

void
virtq_assign_buffer(struct virtio_queue *q)
{
  u16 head = q -> free_head;

  q -> desc[head].flags = 0;
  q -> desc[head].addr = (size_t)kzmalloc(100);
  q -> desc[head].len = 100;
  q -> free_head = q -> desc[head].next;

  u16 avail_idx = (q -> avail -> idx + q -> num_added) % q -> size;
  q -> num_added++;

  q -> avail -> ring[avail_idx] = head;
}