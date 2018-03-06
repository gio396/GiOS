#ifndef __VIRTIO_QUEUE_H__
#define __VIRTIO_QUEUE_H__

#include <common.h>
#include <list.h>

#define VQ_IN 1
#define VQ_OUT 0

struct scatter_list
{
  u8  *buffer;
  u32 len;

  struct dlist_node node;
};

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

  i8 *name;

  u16 last_index_used;
  u16 last_available_index;
  u32 chunk_size;
  u16 next_buffer;
  u16 free_head;
  u16 num_added;
  u16 size;
  u16 last_buffer_seen;

  struct virtq_desc  *desc;
  struct virtq_avail *avail;
  struct virtq_used  *used;

  void(*handle_input)(struct virtio_queue*, struct scatter_list*);
};

struct virtio_queue*
virtio_create_queue(i8 *name, u32 size);

void
virtio_queue_enqueue(struct virtio_queue* q, u8 *buffer, size_t len, u8 direction);

struct scatter_list
virtio_queue_dequeue(struct virtio_queue *q); 

void
virtio_queue_kick(struct virtio_queue *q, u32 iobase);

void
virtio_queue_notify(struct virtio_queue *q, u32 iobase);

void
virtq_assign_buffer(struct virtio_queue *q);

#endif