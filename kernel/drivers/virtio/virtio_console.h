#ifndef __VIRTIO_CONSOLE_H__
#define __VIRTIO_CONSOLE_H__

#include <common.h>

#include "virtio.h"

struct virtio_console
{
  struct virtio_dev *vdev;
//specific.
};

b8
init_vdev_console(struct virtio_dev *vdev);

void
vdev_console_write(struct virtio_console *cdev, u32 port, u8 *buffer, size_t len);

#endif