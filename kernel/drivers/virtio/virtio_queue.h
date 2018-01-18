#ifndef __VIRTIO_QUEUE_H__
#define __VIRTIO_QUEUE_H__

#include <common.h>
#include <list.h>

#include "virtio.h"

struct virtio_queue
{
};

struct virtio_queue*
virtio_create_queue(i8 *name, u32 size);

#endif