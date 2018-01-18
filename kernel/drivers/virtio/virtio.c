#include "virtio.h"

#include <common.h>
#include <macros.h>
#include <list.h>

#include <arch/x86/io.h>
#include <arch/x86/pci.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/page.h>
  
#define VIRTIO_SUBSYSTEM_NETWORK  1
#define VIRTIO_SUBSYSTEM_BLOCK  2
#define VIRTIO_SUBSYSTEM_CHARDEV  3

#include "virtio_console.h"

#define ERINIT -1

#define CHECK_F(fin, feat)                                    \
    do{                                                       \
        b32 __avail = (((fin) & (1 << (feat))) != 0);         \
        LOG(#feat " is%savailable\n", __avail ? " ":" NOT "); \
    }while(0)                                                 \


#define VIRTIO_HEADER_SET_GENERIC(size, name)                            \
    void virtio_header_set_##name(u32 iobase, u32 offset, u8 *buffer)    \
    {                                                                    \
      if (size == 8)                                                     \
      {                                                                  \
        outb(iobase + offset, *buffer);                                  \
      }                                                                  \
      else if (size == 16)                                               \
      {                                                                  \
        outs(iobase + offset, *(u16*)(buffer));                          \
      }                                                                  \
      else if  (size == 32)                                              \
      {                                                                  \
        outl(iobase + offset, *(u32*)(buffer));                          \
      }                                                                  \
      else if (size > 32)                                                \
      {                                                                  \
        assert1((size & 31) == 0);                                       \
        for (i32 i = 0; i < size; i+=32)                                 \
        {                                                                \
          outl(iobase + offset + i, *(u32*)(buffer + i));                \
        }                                                                \
      }                                                                  \
    }                                                                    \

#define VIRTIO_HEADER_GET_GENERIC(size, name)                            \
    u32 virtio_header_get_##name(u32 iobase, u32 offset)                 \
    {                                                                    \
      if (size == 8)                                                     \
      {                                                                  \
        return inb(iobase + offset);                                     \
      }                                                                  \
      else if (size == 16)                                               \
      {                                                                  \
        return ins(iobase + offset);                                     \
      }                                                                  \
      else if  (size == 32)                                              \
      {                                                                  \
        return inl(iobase + offset);                                     \
      }                                                                  \
                                                                         \
      return 0x0;                                                        \
    }                                                                    \

VIRTIO_HEADER_SET_GENERIC(8,  byte)
VIRTIO_HEADER_SET_GENERIC(16, word)
VIRTIO_HEADER_SET_GENERIC(32, dword)
VIRTIO_HEADER_SET_GENERIC(64, qword)
VIRTIO_HEADER_GET_GENERIC(8,  byte)
VIRTIO_HEADER_GET_GENERIC(16, word)
VIRTIO_HEADER_GET_GENERIC(32, dword)

void
virtio_add_status(u32 iobase, u8 ns)
{
  u32 status = virtio_header_get_byte(iobase, OFFSET_OF(struct virtio_header, device_status)) &0xff;
  LOG("Current status 0x%02x\n", status);
  status |= ns;
  virtio_header_set_byte(iobase, OFFSET_OF(struct virtio_header, device_status), (u8*)&status);
}

void
virtio_get_config(u32 iobase, u32 offset, u8 *buffer, size_t size)
{
  u8 *it = buffer;
  for (i32 i = 0; i < size; i++)
  {
    it[i] = virtio_header_get_byte(iobase, offset + i);
  }
}

// u8
// virtio_install()
// {
//   struct dlist_node *it = NULL;
//   struct pci_dev *dev;

//   it = pci_lookup_device_next(it, VIRTIO_VENDOR_ID, VIRTIO_CHAR_DEVICE_ID, VIRTIO_CHAR_DEVICE_SUBSYSTEM_ID);

//   if (it == NULL)
//   {
//     return ERINIT;
//   }

//   dev = CONTAINER_OF(it, struct pci_dev, self);
//   LOG("Found Valid virtio device!\n");
//   LOG("Vendort id = 0x%04x\n", dev -> vendor_id);
//   LOG("capabilities = 0x%02x\n", dev -> capabilities);

//   u32 iobase = 0;

//   iobase = pci_dev_get_iobase(dev);

//   virtio_add_status(iobase, VIRTIO_STATUS_ACK);
//   virtio_add_status(iobase, VIRTIO_STATUS_DRI);

//   u32 features = virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, device_features));

//   LOG("FEATURES = %08x!\n", features);

//   CHECK_F(features, VIRTIO_CHARDEV_F_SIZE);
//   CHECK_F(features, VIRTIO_CHARDEV_F_MULTIPORT);
//   CHECK_F(features, VIRTIO_CHARDEV_F_EMERG_WRITE);

//   virtio_header_set_dword(iobase, OFFSET_OF(struct virtio_header, guest_features), (u8*)&features);

//   u32 features1 = virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, device_features));

//   if (features == features1)
//   {
//     LOG("FEATURE NAGOTIATION SUCCSESS!\n");
//   }
//   else
//   {
//     return 0;
//   }

//   virtio_add_status(iobase, VIRTIO_STATUS_DRI_OK);

//   u16 queue_size = virtio_header_get_word(iobase, OFFSET_OF(struct virtio_header, queue_size));

//   LOG("QUEUE SIZE = %x\n", queue_size);

//   struct virtio_chardev_header header = {};
//   u32 offset = sizeof(struct virtio_header);

//   if (dev -> msix != 0)
//     offset += sizeof(struct virtio_msix_header);

//   virtio_get_config(iobase, offset, (u8*)&header, sizeof(struct virtio_chardev_header));

//   LOG("CHARDEV %u %u %u\n", header.rows, header.cols, header.max_n_ports);

//   u32 zero = 0;
//   virtio_header_set_word(iobase, OFFSET_OF(struct virtio_header, queue_select), (u8*)&zero);
//   u32 qaddr = virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, queue_size));

//   LOG("VIRTQUEUE SIZE %d\n", qaddr);

//   return 1;
// }

void
init_vdev_common(struct pci_dev *dev, struct virtio_dev *vdev)
{
  vdev -> iobase = pci_dev_get_iobase(dev);
}

u8
virtio_install()
{
  struct dlist_node *it = NULL;
  struct pci_dev   *dev;

  it = pci_lookup_device(it, VIRTIO_VENDOR_ID);
  while (it !=  NULL)
  {
    dev = CONTAINER_OF(it, struct pci_dev, self);
    if (dev -> device_id < 0x1000  && dev -> device_id > 0x1005)
      goto nextl;

    LOG("Got valid virtio device with vendor id 0x%04x and device id 0x%04x\n",
       dev -> vendor_id, dev -> device_id);

    struct virtio_dev vdev = {};
    init_vdev_common(dev, &vdev);

    switch (dev -> subsystem_id)
    {
      case VIRTIO_SUBSYSTEM_CHARDEV:
      {
        init_vdev_console(&vdev);
      }break;

      case VIRTIO_SUBSYSTEM_BLOCK:
      {
      }break;

      case VIRTIO_SUBSYSTEM_NETWORK:
      {
      }break;

      default:
      {
        goto nextl;
      }
    }

  nextl:
    it = pci_lookup_device_next(it, VIRTIO_VENDOR_ID);
  }

  return 1;
}