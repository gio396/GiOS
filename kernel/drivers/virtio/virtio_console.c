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

#define VIRTQ_DESC_F_WRITE 2

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

struct virtio_console*
vdev_to_cdev(struct virtio_dev *vdev)
{
  return CONTAINER_OF(vdev, struct virtio_console, vdev);
}

void
console_send_control_message(struct virtio_console *cdev, u32 id, u16 event, u16 value)
{
  struct virtio_console_control *ctrl = (struct virtio_console_control*)kzmalloc(sizeof(struct virtio_console_control));

  ctrl -> id = id;
  ctrl -> event = event;
  ctrl -> value = value;

  struct virtio_queue *vq = virtio_get_queue(&cdev -> vdev, 3);
  virtio_queue_enqueue(vq, (u8*)ctrl, sizeof(struct virtio_console_control), VQ_OUT);
  virtio_dev_kick_queue(&cdev -> vdev, vq);
}

void
console_handle_control_message(struct virtio_queue *q, struct scatter_list *list)
{
  u8 *buffer = list -> buffer;
  struct virtio_console_control *cmsg = (struct virtio_console_control*)buffer;
  struct virtio_console *cdev = vdev_to_cdev(q -> vdev);
  assert1(cmsg -> id < q -> size);

  LOGV("%d", cmsg -> id);
  LOGV("%d", cmsg -> event);
  LOGV("%d", cmsg -> value);

  switch (cmsg -> event)
  {
    case VIRTIO_CONSOLE_DEVICE_ADD:
    {
      //request to add port with id cmsg -> id
      LOG("Recieved request to add port device #%d\n", cmsg -> id);
      u16 base_id = RN(cmsg -> id);
      virtio_set_queue(&cdev -> vdev, base_id, 
          virtio_create_queue("NULL", virtio_get_queue_size(&cdev -> vdev, base_id)));
      virtio_set_queue(&cdev -> vdev, base_id + 1,
          virtio_create_queue("NULL", virtio_get_queue_size(&cdev -> vdev, base_id + 1)));


      console_send_control_message(cdev, cmsg -> id, VIRTIO_CONSOLE_PORT_READY, 1);
    }break;

    case VIRTIO_CONSOLE_PORT_NAME:
    {
      LOGV("Recieved port name from device for port #%d\n", cmsg -> id);
      LOGV("%d", list -> len);
      u16 name_len = list -> len - sizeof(struct virtio_console_control);
      u8 name[name_len + 1];
      memcpy(buffer + sizeof(struct virtio_console_control), name,  name_len);
      name[name_len] = '\0';

      LOG("PORT_NAME = %s\n", name);
    }break;

    case VIRTIO_CONSOLE_PORT_OPEN:
    {
      LOGV("Recieved request to open port #%d\n", cmsg -> id);
      console_send_control_message(cdev, cmsg -> id, VIRTIO_CONSOLE_PORT_OPEN, 1);
    }break;
  }
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
  u8 *sb = (u8*)kzmalloc(6);

  memcpy("kata\r\n", sb, 6);

  vdev_console_write(cdev, 1, sb, 9);

  struct virtio_queue *q = virtio_get_queue(&cdev -> vdev, 2);
  LOGV("%d", q -> used -> idx);
  LOGV("%d", q -> avail -> idx);

  u16 ring = q -> next_buffer % q -> size;
  u16 id = q -> used -> ring[ring].id;
  LOGV("%d", id);

  u8 *buffer = (u8*)(size_t)q -> desc[id].addr;
  q -> next_buffer++;

  LOGV("%p", buffer);
  LOGV("%d", q -> used -> ring[ring].len);

  struct virtio_console_control *cmsg = (struct virtio_console_control*)buffer;
  LOGV("%d", cmsg ->  id);
  LOGV("%d", cmsg ->  event);
  LOGV("%d", cmsg ->  value);

  u8 isr = virtio_get_isr(&cdev -> vdev);
  LOGV("%02x", isr);
}

void
setup_rx(struct virtio_console *cdev, u32 vqid)
{
  struct virtio_queue *q = virtio_get_queue(&cdev -> vdev, vqid);
  u32 iobase = cdev -> vdev.iobase;
  virtio_queue_enqueue(q, kzmalloc(0x400), 400, VQ_IN);
  virtio_queue_kick(q, iobase);
  virtio_queue_enqueue(q, kzmalloc(0x400), 400, VQ_IN);
  virtio_queue_kick(q, iobase);
  virtio_queue_enqueue(q, kzmalloc(0x400), 400, VQ_IN);
  virtio_queue_kick(q, iobase);
  virtio_queue_enqueue(q, kzmalloc(0x400), 400, VQ_IN);
  virtio_queue_kick(q, iobase);
  virtio_queue_enqueue(q, kzmalloc(0x400), 400, VQ_IN);
  virtio_queue_kick(q, iobase);
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

  struct virtio_queue *cr0 = virtio_create_queue("cr0", virtio_get_queue_size(&cdev -> vdev, 2));
  cr0 -> handle_input = console_handle_control_message;

  virtio_set_queue(&cdev -> vdev, 2, 
    cr0);
  virtio_set_queue(&cdev -> vdev, 3, 
    virtio_create_queue("ct0", virtio_get_queue_size(&cdev -> vdev, 3)));

  setup_rx(cdev, 2);

  console_send_control_message(cdev, 0xffffffff, VIRTIO_CONSOLE_DEVICE_READY, 1);
  new_timer(5 * 1000 * 1000, write_later, (u32)cdev);
  new_timer(5 * 1000 * 1000, write_later, (u32)cdev);
  new_timer(5 * 1000 * 1000, write_later, (u32)cdev);
  new_timer(5 * 1000 * 1000, write_later, (u32)cdev);
  

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
  UNUSED(cdev);
  u8 isr = virtio_get_isr(vdev);

  if ((isr & 0x1) == 0x1)
  {
    LOG("VIRTQ CHANGE\n");
    for (i32 i = 0; i < 32; i++)
    {
      struct virtio_queue *q = vdev -> virtq[i];
      if (q)
      {
        LOG("%d %d %d\n", q -> idx, q -> last_buffer_seen, q -> used -> idx);
        while (q -> last_buffer_seen < q -> used -> idx)
        {
          LOG("input!\n");

          struct scatter_list list = virtio_queue_dequeue(q);
          q -> handle_input(q, &list);
        }
      }
    }
  }
  if ((isr & 0x3) == 0x3)
  {
    LOG("CONFIGURATION CHANGE!\n");
    //handle configuration change
  }
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
  virtio_queue_enqueue(q, buffer, len, VQ_OUT);
  virtio_dev_kick_queue(&cdev -> vdev, q);
  LOG("USED IDX! %d\n", q -> used -> idx);

  q = virtio_get_queue(&cdev -> vdev, 2);
  u16 status = pci_get_status(&cdev -> vdev.pdev);
  LOG("STATUS = %016b\n", status);
}
