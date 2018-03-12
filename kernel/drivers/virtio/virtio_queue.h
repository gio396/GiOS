#ifndef __VIRTIO_QUEUE_H__
#define __VIRTIO_QUEUE_H__

#include <common.h>
#include <list.h>
#include <scatterlist.h>

#define VQ_IN 1
#define VQ_OUT 0

struct virtq_desc
{
  u64 addr;
  u32 len;

  u16 flags;
  u16 next;
};

struct virtq_avail
{
  u16 flags;
  u16 idx;
  u16 ring[];
  //implicit u16 used_event at the end if VIRITO_F_EVENT_IDX is pressent!
};

struct virtq_used_elem
{
  u32 id;
  u32 len;
};

struct virtq_used
{
  u16 flags;
  u16 idx;
  struct virtq_used_elem ring[];
  //implicit u16 used_event at the end if VIRTIO_F_EVENT_IDX is pressent!
};

struct virtio_queue
{
  struct virtio_dev *vdev;
  u32 idx;

  u16 next_buffer;
  u16 num_added;
  u16 size;
  u16 last_buffer_seen;

  struct virtq_desc  *desc;
  struct virtq_avail *avail;
  struct virtq_used  *used;

  void(*handle_input)(struct virtio_queue*, struct scatterlist*);
};

struct virtio_queue*
virtio_create_queue(u32 size);

void
virtio_queue_enqueue(struct virtio_queue* q, u8 *buffer, size_t last_buffer_seen, u8 direction);

struct scatterlist
virtio_queue_dequeue(struct virtio_queue *q); 

void
virtio_queue_kick(struct virtio_queue *q);

void
virtio_queue_notify(struct virtio_queue *q);

void
virtq_assign_buffer(struct virtio_queue *q);

i8
virtio_queue_has_unseen_buffers(struct virtio_queue *q);

#endif