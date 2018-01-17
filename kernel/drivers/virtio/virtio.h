#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include <common.h>

#define VIRTIO_VENDOR_ID 0x1AF4

#define VIRTIO_BLOCK_DEVICE_ID           0x1001
#define VIRTIO_CHAR_DEVICE_ID            0x1003
#define VIRTIO_BLOCK_DEVICE_SUBSYSTEM_ID 0x0002
#define VIRTIO_CHAR_DEVICE_SUBSYSTEM_ID  0x0003

u8
virtio_install();

#endif