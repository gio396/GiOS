#ifndef __VIRTIO_CONSOLE_H__
#define __VIRTIO_CONSOLE_H__

#include <common.h>

#include "virtio.h"

struct virtio_console
{
  struct virtio_dev vdev;
//specific.
};

b8
init_vdev_console(struct virtio_dev *vdev);

#endif