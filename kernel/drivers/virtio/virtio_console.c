#include "virtio_console.h"

#include "virtio_queue.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/idt.h>

#include <drivers/pci/pci.h>

#include <macros.h>
#include <list.h>
#include <timer.h>
#include <string.h>
#include <memory.h>

#define R0  0
#define W0  1
#define CR0 2
#define CW0 3
#define RN(n) (4 + (2 * ((n) - 1)))
#define WN(n) (RN(n) + 1)

#define VIRTIO_CONSOLE_F_SIZE          0
#define VIRTIO_CONSOLE_F_MULTIPORT     1
#define VIRTIO_CONSOLE_F_EMERG_WRITE   2

#define VIRTIO_CONSOLE_DEVICE_READY    0
#define VIRTIO_CONSOLE_DEVICE_ADD      1
#define VIRTIO_CONSOLE_DEVICE_REMOVE   2
#define VIRTIO_CONSOLE_PORT_READY      3
#define VIRTIO_CONSOLE_CONSOLE_PORT    4
#define VIRTIO_CONSOLE_RESIZE          5
#define VIRTIO_CONSOLE_PORT_OPEN       6
#define VIRTIO_CONSOLE_PORT_NAME       7

struct virtio_console_control
{
  u32 id;
  u16 event;
  u16 value;
};

  
#define CHECK_F(fin, feat)                                    \
    do{                                                       \
        b32 __avail = (((fin) & (1 << (feat))) != 0);         \
        LOG(#feat " is%savailable\n", __avail ? " ":" NOT "); \
    }while(0)                                                 \

struct virtio_console*
new_console_device(struct pci_dev *pdev)
{
  struct virtio_console *cdev = (struct virtio_console*)kzmalloc(sizeof(struct virtio_console));

  return cdev;
}

void intr_handler()
{
  
}

struct virtio_console*
vdev_to_cdev(struct virtio_dev *vdev)
{
  return CONTAINER_OF(vdev, struct virtio_console, vdev);
}

b8
console_features(struct virtio_dev *vdev, u32 features)
{
  LOG("Console got features %b\n", vdev -> features);
  CHECK_F(features, VIRTIO_CONSOLE_F_SIZE);
  CHECK_F(features, VIRTIO_CONSOLE_F_MULTIPORT);
  CHECK_F(features, VIRTIO_CONSOLE_F_EMERG_WRITE);

  u32 mandatory_features = (1 << VIRTIO_CONSOLE_F_MULTIPORT);

  return vdev_confirm_features(vdev, mandatory_features);
}

void
write_later(u32 addr)
{
  struct virtio_console *cdev = (struct virtio_console*)(addr);
  vdev_console_write(cdev, 0, (u8*)"kata\r\n", 7);

  struct virtio_queue *q = virtio_get_queue(&cdev -> vdev, 2);
  LOGV("%d", q -> used -> idx);
  LOGV("%d", q -> avail -> idx);
}

void
setup_rx(struct virtio_console *cdev, u32 vqid)
{
  struct virtio_queue *q = virtio_get_queue(&cdev -> vdev, vqid);
  u32 head = q -> free_head;

  LOGV("%d", head);
  LOGV("%p", q -> desc);

  q -> desc[head].addr = (u32)kzmalloc(0x400);
  q -> desc[head].len  = 0x400;
  q -> desc[head].flags = 0;
  q -> free_head = q -> desc[head].next;

  q -> avail -> ring[(q -> avail -> idx + q -> num_added) % q -> size] = head;
  q -> num_added++;

  u32 iobase = cdev -> vdev.iobase;
  virtio_queue_notify(q, iobase);

  LOGV("%p", q -> avail -> flags);
}

void
console_send_control_message(struct virtio_console *cdev, u32 id, u16 event, u16 value)
{
  struct virtio_console_control *ctrl = (struct virtio_console_control*)kzmalloc(sizeof(struct virtio_console_control));

  ctrl -> id = id;
  ctrl -> event = event;
  ctrl -> value = value;

  struct virtio_queue *vq = virtio_get_queue(&cdev -> vdev, 3);
  virtio_queue_enqueue(vq, (u8*)ctrl, sizeof(struct virtio_console_control));
  virtio_dev_kick_queue(&cdev -> vdev, vq);
  LOG("USED IDX! %d\n", vq -> used -> idx);
}

b8
console_setup(struct virtio_dev *vdev)
{
  struct virtio_console *cdev = vdev_to_cdev(vdev);

  virtio_read_config(vdev, sizeof(struct virtio_console_config), (u8*)&cdev -> cfg);
  LOGV("%d", cdev -> cfg.cols);
  LOGV("%d", cdev -> cfg.rows);
  LOGV("%d", cdev -> cfg.nr_ports);
  LOGV("%d", cdev -> cfg.emerg_wr);

  virtio_set_queue(&cdev -> vdev, 0, 
    virtio_create_queue("r0",  virtio_get_queue_size(&cdev -> vdev, 0)));
  virtio_set_queue(&cdev -> vdev, 1, 
    virtio_create_queue("t0",  virtio_get_queue_size(&cdev -> vdev, 1)));
  virtio_set_queue(&cdev -> vdev, 2, 
    virtio_create_queue("cr0", virtio_get_queue_size(&cdev -> vdev, 2)));
  virtio_set_queue(&cdev -> vdev, 3, 
    virtio_create_queue("ct0", virtio_get_queue_size(&cdev -> vdev, 3)));

  setup_rx(cdev, 0);
  setup_rx(cdev, 2);

  console_send_control_message(cdev, 0xffffffff, VIRTIO_CONSOLE_DEVICE_READY, 1);


  new_timer(5 * 1000 * 1000, write_later, (u32)cdev);
  new_timer(6 * 1000 * 1000, write_later, (u32)cdev);
  new_timer(7 * 1000 * 1000, write_later, (u32)cdev);
  new_timer(8 * 1000 * 1000, write_later, (u32)cdev);

  return 1;
}

void
console_remove(struct virtio_dev *vdev)
{
  struct virtio_console *cdev = vdev_to_cdev(vdev);
  UNUSED(cdev);
  //kfree(cdev);

  return;
}

void
console_interrupt(const union biosregs *iregs, struct virtio_dev *vdev)
{
  struct virtio_console *cdev = vdev_to_cdev(vdev);
  LOG("INTERUPTING CONSOLE! %p\n", cdev);
}

struct virtio_driver virtio_console_driver = {
  .probe_features = console_features,
  .setup = console_setup,
  .ievent = console_interrupt,
};


struct virtio_dev*
init_vdev_console(struct pci_dev *dev)
{
  LOG("Initializing virtio console(console)\n");
  struct virtio_console *cdev = new_console_device(dev);

  cdev -> vdev.driver = &virtio_console_driver;
  return &cdev -> vdev;
}

void
vdev_console_write(struct virtio_console *cdev, u32 port, u8 *buffer, size_t len)
{
  i32 qidx = 1;
  if (port > 0)
  {
    qidx = WN(port);
  }

  struct virtio_queue *q = virtio_get_queue(&cdev -> vdev, qidx);
  virtio_queue_enqueue(q, buffer, len);
  virtio_dev_kick_queue(&cdev -> vdev, q);
  LOG("USED IDX! %d\n", q -> used -> idx);

  q = virtio_get_queue(&cdev -> vdev, 2);
  u16 status = pci_get_status(&cdev -> vdev.pdev);
  LOG("STATUS = %016b\n", status);
}