#ifndef __VIRTIO_CONSOLE_H__
#define __VIRTIO_CONSOLE_H__

#include <common.h>

#include "virtio.h"

struct virtio_console_config
{
  u16 cols;
  u16 rows;
  u32 nr_ports;
  u32 emerg_wr;
};

struct virtio_console
{
  struct virtio_dev vdev;
  struct virtio_console_config cfg;
};

struct virtio_dev*
init_vdev_console(struct pci_dev *dev);

void
vdev_console_write(struct virtio_console *cdev, u32 port, u8 *buffer, size_t len);

#endif