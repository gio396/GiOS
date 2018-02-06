#include "virtio_console.h"

#include "virtio_queue.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/idt.h>

#include <drivers/pci/pci.h>

#include <macros.h>
#include <list.h>
#include <timer.h>
#include <string.h>

#define R0  0
#define W0  1
#define CR0 2
#define CW0 3
#define RN(n) (4 + (2 * ((n) - 1)))
#define WN(n) (RN(n) + 1)


#define VIRTIO_console_F_SIZE          0
#define VIRTIO_console_F_MULTIPORT     1
#define VIRTIO_console_F_EMERG_WRITE   2

#define VIRTIO_console_DEVICE_READY    0
#define VIRTIO_console_DEVICE_ADD      1
#define VIRTIO_console_DEVICE_REMOVE   2
#define VIRTIO_console_PORT_READY      3
#define VIRTIO_console_console_PORT    4
#define VIRTIO_console_RESIZE          5
#define VIRTIO_console_PORT_OPEN       6
#define VIRTIO_console_PORT_NAME       7

  
#define CHECK_F(fin, feat)                                    \
    do{                                                       \
        b32 __avail = (((fin) & (1 << (feat))) != 0);         \
        LOG(#feat " is%savailable\n", __avail ? " ":" NOT "); \
    }while(0)                                                 \

u32 console_head = 0;
struct virtio_console console_stack[16];

struct virtio_console*
new_console(struct virtio_dev *vdev)
{
  UNUSED(vdev);
  struct virtio_console *cdev = &console_stack[console_head++];
  cdev -> vdev = vdev;
  return cdev;
}

void intr_handler()
{

}

b8
init_vdev_console(struct virtio_dev *vdev)
{
  UNUSED(vdev);
  LOG("Initializing virtio console(console)\n");
  struct virtio_console *cdev = new_console(vdev);
  UNUSED(cdev);

  u32 iobase = vdev -> iobase;
  virtio_add_status(iobase, VIRTIO_STATUS_ACK);
  virtio_add_status(iobase, VIRTIO_STATUS_DRI);

  u32 features = virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, device_features));
  u32 features1;

  LOG("Features = %b\n", features);
  CHECK_F(features, VIRTIO_console_F_SIZE);
  CHECK_F(features, VIRTIO_console_F_MULTIPORT);
  CHECK_F(features, VIRTIO_console_F_EMERG_WRITE);

  features |= (1 << VIRTIO_console_F_MULTIPORT);
  virtio_header_set_dword(iobase, OFFSET_OF(struct virtio_header, guest_features), (u8*)&features);

  features1 = virtio_header_get_dword(iobase, OFFSET_OF(struct virtio_header, device_features));

  if (features != features1)
  {
    LOG("Features nagotiation failed!\n");
    virtio_add_status(iobase, VIRTIO_STATUS_FAILED);

    return 0;
  }

  virtio_read_config(cdev -> vdev, sizeof(struct virtio_console_config), (u8*)&cdev -> cfg);

  LOGV("%d", cdev -> cfg.cols);
  LOGV("%d", cdev -> cfg.rows);
  LOGV("%d", cdev -> cfg.nr_ports);
  LOGV("%d", cdev -> cfg.emerg_wr);

  virtio_add_status(iobase, VIRTIO_STATUS_FEATURES_OK);

  virtio_set_queue(cdev -> vdev, 0, 
    virtio_create_queue("r0",  virtio_get_queue_size(cdev -> vdev, 0)));
  virtio_set_queue(cdev -> vdev, 1, 
    virtio_create_queue("t0",  virtio_get_queue_size(cdev -> vdev, 1)));
  virtio_set_queue(cdev -> vdev, 2, 
    virtio_create_queue("cr0", virtio_get_queue_size(cdev -> vdev, 2)));
  virtio_set_queue(cdev -> vdev, 3, 
    virtio_create_queue("ct0", virtio_get_queue_size(cdev -> vdev, 3)));

  struct virtio_queue *q = virtio_get_queue(cdev -> vdev, 0);
  virtq_assign_buffer(q);
  virtio_dev_kick_queue(cdev -> vdev, q);

  struct pci_dev *pdev = cdev -> vdev -> pci_dev;
  if (pdev -> has_msix)
  {
    pci_msix_enable(pdev);
    for (u32 i = 0; i < pdev -> msix.max_entries; i++)
    {
      u32 irq = get_next_irq();
      subscribe_irq(irq, intr_handler);
      msi_set_vector(&pdev -> msix, 0, irq);
    }
  }

  virtio_add_status(iobase, VIRTIO_STATUS_DRI_OK);

  while(1)
  {
    vdev_console_write(cdev, 0, (u8*)"kata\n\r", 7);
    sleep(100);
  }

  return 1;
}

void
vdev_console_write(struct virtio_console *cdev, u32 port, u8 *buffer, size_t len)
{
  i32 qidx = 1;
  if (port > 0)
  {
    qidx = WN(port);
  }

  struct virtio_queue *q = virtio_get_queue(cdev -> vdev, qidx);
  virtio_queue_enqueue(q, buffer, len);
  virtio_dev_kick_queue(cdev -> vdev, q);
  LOG("USED IDX! %d\n", q -> used -> idx);
}