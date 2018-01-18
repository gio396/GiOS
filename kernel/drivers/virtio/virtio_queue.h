#ifndef __VIRTIO_QUEUE_H__
#define __VIRTIO_QUEUE_H__

#include <common.h>
#include <list.h>

#include "virtio.h"

struct virtq_desc
{
  u64 addr;
  u32 len;

  u16 flags;
  u16 next;
} att_packed;

struct virtq_avail
{
  u16 flags;
  u16 idx;
  u16 ring[];
  //implicit u16 used_event at the end if VIRITO_F_EVENT_IDX is pressent!
} att_packed;

struct virtq_used_elem
{
  u32 id;
  u32 len;
} att_packed;

struct virtq_used
{
  u16 flags;
  u16 idx;
  struct virtq_used_elem ring[];
  //implicit u16 used_event at the end if VIRTIO_F_EVENT_IDX is pressent!
} att_packed;

struct virtio_queue
{
  i8 *name;
  u32 idx;

  struct virtq_desc  *desc;
  struct virtq_avail *avail;
  struct virtq_used  *used;
};

struct virtio_queue*
virtio_create_queue(i8 *name, u32 size);

#endif