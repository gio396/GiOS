#include "virtio.h"

#include <common.h>
#include <macros.h>
#include <list.h>

#include <arch/x86/io.h>
#include <drivers/pci/pci.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/idt.h>
#include <arch/x86/page.h>
  
#define VIRTIO_SUBSYSTEM_NETWORK  1
#define VIRTIO_SUBSYSTEM_BLOCK  2
#define VIRTIO_SUBSYSTEM_CONSOLE  3

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

struct virtio_dev *
pdev_to_vdev(struct pci_dev *pdev)
{
  return CONTAINER_OF(pdev, struct virtio_dev, pdev);
}

void
virtio_add_status(u32 iobase, u8 ns)
{
  u32 status = virtio_header_get_byte(iobase, OFFSET_OF(struct virtio_header, device_status)) & 0xff;

  status |= (1 << ns);

  virtio_header_set_byte(iobase, OFFSET_OF(struct virtio_header, device_status), (u8*)&status);

  status = virtio_header_get_byte(iobase, OFFSET_OF(struct virtio_header, device_status)) & 0xff;

  LOG("STATUS %02x!\n", status);
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

void
virtio_set_queue(struct virtio_dev *dev, i32 idx, struct virtio_queue *que)
{
  dev -> virtq[idx]        = que;
  dev -> virtq[idx] -> idx = idx;

  u32 iobase = dev -> iobase;
  u32 uaddr = (u32)que -> desc / 4096;
  
  virtio_header_set_word(iobase, OFFSET_OF(struct virtio_header, queue_select), (u8*)&idx);
  virtio_header_set_dword(iobase, OFFSET_OF(struct virtio_header, queue_addr), (u8*)&uaddr);

  u32 addr = virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, queue_addr));

  if (addr == uaddr)
  {
    LOG("Successfully set virtq queue!\n");
  }

  if (dev -> pdev.msix.enabled)
  {
    u16 vector = 0;
    virtio_header_set_word(iobase, OFFSET_OF(struct virtio_header, config_msix_vector), (u8*)&vector);

    vector = 1;
    virtio_header_set_word(iobase, OFFSET_OF(struct virtio_header, queue_msix_vector), (u8*)&vector);

  }
}


struct virtio_queue*
virtio_get_queue(struct virtio_dev *dev, i32 idx)
{
  return dev -> virtq[idx];
}

u32
virtio_get_queue_size(struct virtio_dev *dev, i32 idx)
{
  u32 iobase = dev -> iobase;
  virtio_header_set_word(iobase, OFFSET_OF(struct virtio_header, queue_select), (u8*)&idx);

  u32 size = virtio_header_get_word(iobase, OFFSET_OF(struct virtio_header, queue_size));

  return size;
}

void
virtio_dev_kick_queue(struct virtio_dev *dev, struct virtio_queue *q)
{
  virtio_queue_kick(q, dev -> iobase);
}

internal size_t
get_config_offset(struct virtio_dev *dev)
{
  size_t offset = sizeof(struct virtio_header);

  if (!dev -> pdev.msix.enabled)
    offset -= VIRTIO_MSIX_HEADER_SIZE;

  return offset;
}

void
virtio_read_config(struct virtio_dev *dev, size_t size, u8* buffer)
{
  size_t offset = get_config_offset(dev);
  size_t iobase = dev -> iobase;

  for (i32 i = 0; i < size; i++)
  {
    buffer[i] = virtio_header_get_byte(iobase, offset  + i);
  }
}

u8
virtio_get_isr(struct virtio_dev *dev)
{
  u8 isr;
  u32 iobase = dev -> iobase;

  isr = virtio_header_get_byte(iobase, OFFSET_OF(struct virtio_header, isr_status));

  return isr;
}

b8
vdev_confirm_features(struct virtio_dev *vdev, u32 features)
{
  u32 iobase = vdev -> iobase;
  virtio_header_set_dword(iobase, OFFSET_OF(struct virtio_header, guest_features), (u8*)&features);

  u32 features1 = virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, device_features));

  return ((features & features1) == features);
}

u32
virtio_get_features(struct virtio_dev *vdev)
{
  u32 iobase = vdev -> iobase;
  return virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, device_features));
}

b8 
virtio_probe(struct pci_dev *dev)
{
  struct virtio_dev *vdev = pdev_to_vdev(dev);
  u32 iobase = vdev -> iobase;

  virtio_add_status(iobase, VIRTIO_STATUS_ACK);
  virtio_add_status(iobase, VIRTIO_STATUS_DRI);

  u32 features = virtio_get_features(vdev);

  struct virtio_driver *vdri = vdev -> driver;

  if (vdri -> probe_features(vdev, features) == 0)
    goto features_fail;

  // skip this step for legacy drivers.
  // virtio_add_status(iobase, VIRTIO_STATUS_FEATURES_OK);

  return 1;

features_fail:
  virtio_add_status(iobase, VIRTIO_STATUS_FAILED);
  return 0;
}

b8
virtio_setup(struct pci_dev *dev)
{
  struct virtio_dev *vdev = pdev_to_vdev(dev);
  struct virtio_driver *vdri = vdev -> driver;

  b8 dri_status = vdri -> setup(vdev);
  if (dri_status)
  {
    u32 iobase = vdev -> iobase;
    virtio_add_status(iobase, VIRTIO_STATUS_DRI_OK);
  }

  return dri_status;
}

b8
virtio_match(struct pci_dev *dev)
{
  //TODO(gio)
  return 1;
}

void
init_vdev_common(struct pci_dev *pdev, struct virtio_dev *vdev)
{
  vdev -> pdev = *pdev;
  vdev -> iobase = pci_dev_get_iobase(&vdev -> pdev);
}

struct pci_dev*
virtio_init(struct pci_dev *dev)
{
  struct virtio_dev *vdev;
  switch (dev -> subsystem_id)
  {
    case VIRTIO_SUBSYSTEM_CONSOLE:
    {
      vdev = init_vdev_console(dev);
    }break;

    default:
      return NULL;
  }

  init_vdev_common(dev, vdev);
  return &vdev -> pdev;
}

void
virtio_remove(struct pci_dev *dev)
{
  struct virtio_dev *vdev = pdev_to_vdev(dev);
  struct virtio_driver *vdri = vdev -> driver;
  vdri -> remove(vdev);
}

void
virtio_interrupt(const union biosregs *iregs, struct pci_dev *dev)
{
  struct virtio_dev *vdev = pdev_to_vdev(dev);
  struct virtio_driver *vdri = vdev -> driver;

  vdri -> ievent(iregs, vdev);
}

struct pci_id id_list[] = {
  {VIRTIO_VENDOR_ID, PCI_DEVICE_ANY, PCI_SUBSYSTEM_ANY, PCI_CLASS_ANY, PCI_SUB_CLASS_ANY, PCI_REVISION_ANY},
  {0, 0, 0, 0, 0, 0}
};

struct pci_driver virtio_pci_driver = {
  .name = ".virtio_pci", 
  .id_list = id_list,

  .match = virtio_match,
  .init  = virtio_init,
  .probe = virtio_probe, 
  .setup = virtio_setup,
  .remove = virtio_remove,
  .ievent = virtio_interrupt,
};

void
virtio_driver_register()
{
  pci_register_driver(&virtio_pci_driver);
  return;
};

INIT_CORE0_EXPORT(virtio_driver_register);
