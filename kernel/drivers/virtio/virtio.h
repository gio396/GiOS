#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include <common.h>
#include <drivers/pci/pci.h>

#include "virtio_queue.h"

#define VIRTIO_VENDOR_ID 0x1AF4

#define VIRTIO_BLOCK_DEVICE_ID           0x1001
#define VIRTIO_CHAR_DEVICE_ID            0x1003
#define VIRTIO_BLOCK_DEVICE_SUBSYSTEM_ID 0x0002
#define VIRTIO_CHAR_DEVICE_SUBSYSTEM_ID  0x0003

#define VIRTIO_MSIX_HEADER_SIZE   0x04

#define VIRTIO_STATUS_ACK         0
#define VIRTIO_STATUS_DRI         1
#define VIRTIO_STATUS_DRI_OK      2
#define VIRTIO_STATUS_FEATURES_OK 3
#define VIRTIO_STATUS_FAILED      6

struct virtio_dev
{
  struct pci_dev pdev;

  u8 devce_type;
  u32 iobase;
  u32 features;

  //driver
  struct virtio_driver *driver;
};

struct virtio_driver
{
  b8 (*probe_features)(struct virtio_dev* vdev, u32 features);
  b8 (*setup)(struct virtio_dev *dev);
  void (*ievent)(const union biosregs *iregs, struct virtio_dev *dev);
  void (*remove)(struct virtio_dev *dev);
};

struct virtio_cap
{
  u8 cap_vndr;    /* Generic PCI field: PCI_CAP_ID_VNDR */
  u8 cap_next;    /* Generic PCI field: next ptr. */
  u8 cap_len;     /* Generic PCI field: capability length */
  u8 cfg_type;    /* Identifies the structure. */
  u8 bar;
  u8 padding[3];
  u32 offset;    /* Offset within bar. */
  u32 length;
};

struct virtio_header
{
  u32 device_features;
  u32 guest_features;
  u32 queue_addr;
  u16 queue_size;
  u16 queue_select;
  u16 queue_notify;
  u8  device_status;
  u8  isr_status;

  //if msix is enabled !
  u16 config_msix_vector;
  u16 queue_msix_vector;
} att_packed;

struct virtio_msix_header
{
  u16 vector;
  u16 queue;
} att_packed;

struct virtio_chardev_header
{
  u16 cols;
  u16 rows;
  u32 max_n_ports;
  u32 emerg_wr;
} att_packed;

struct virtio_chardev_control
{
  u32 id;
  u16 event;
  u16 value;
} att_packed;

u8
virtio_install();

#define VIRTIO_HEADER_GET_GENERIC(size, name)                            \
    u32 virtio_header_get_##name(u32 iobase, u32 offset)                 \

#define VIRTIO_HEADER_SET_GENERIC(size, name)                            \
    void virtio_header_set_##name(u32 iobase, u32 offset, u8 *buffer)    \

VIRTIO_HEADER_SET_GENERIC(8,  byte);
VIRTIO_HEADER_SET_GENERIC(16, word);
VIRTIO_HEADER_SET_GENERIC(32, dword);
VIRTIO_HEADER_SET_GENERIC(64, qword);
VIRTIO_HEADER_GET_GENERIC(8,  byte);
VIRTIO_HEADER_GET_GENERIC(16, word);
VIRTIO_HEADER_GET_GENERIC(32, dword);

#undef VIRTIO_HEADER_SET_GENERIC
#undef VIRTIO_HEADER_GET_GENERIC

void
virtio_add_status(u32 iobase, u8 ns);

void
virtio_set_queue(struct virtio_dev *dev, i32 idx, struct virtio_queue *que);

u32
virtio_get_queue_size(struct virtio_dev *dev, i32 idx);

void
virtio_read_config(struct virtio_dev *dev, size_t size, u8* buffer);

b8
vdev_confirm_features(struct virtio_dev *vdev, u32 features);

u8
virtio_get_isr(struct virtio_dev *vdev);

#endif