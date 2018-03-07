#ifndef __VIRTIO_BLOCK_H__
#define __VIRTIO_BLOCK_H__

#include <common.h>
#include "virtio.h"

struct virtio_dev*
init_vdev_block(struct pci_dev *dev);

// void
// vdev_console_write(struct virtio_console *cdev, u32 port, u8 *buffer, size_t len);

#endif