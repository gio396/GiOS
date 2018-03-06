#include "virtio_console.h"

#include "virtio_queue.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/idt.h>
#include <arch/x86/page.h>


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

#define VIRTIO_PORT_IS_CONSOLE (1 << 0)
#define VIRTIO_PORT_OPEN       (1 << 1)

struct port_buffer
{
  u8 *base;
  u32 len;
};

void
init_port_buffer(struct port_buffer *buffer)
{
  buffer -> base = kalloc();
  buffer -> len = kb(4);
}

struct virtio_port
{
  struct virtio *vdev;

  struct port_buffer inbuf;
  struct port_buffer outbuf;

  struct virtio_queue *vq_in, *vq_out;

  //utf-8 name for port
  u8 *name;
  u8 flags;
  u16 id;

  struct dlist_node node;
};

struct virtio_console
{
  struct virtio_dev vdev;
  struct virtio_console_config cfg;

  u16 num_ports;
  struct virtio_port cmsg_port;
  struct virtio_port port;
};

void
vdev_console_write(struct virtio_console *cdev, u32 port, u8 *buffer, size_t len);

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
  virtio_queue_enqueue(q, kzmalloc(0x400), 400, VQ_IN);
  virtio_queue_kick(q, q -> vdev -> iobase);

  u8 *buffer = list -> buffer;
  struct virtio_console_control *cmsg = (struct virtio_console_control*)buffer;
  struct virtio_console *cdev = vdev_to_cdev(q -> vdev);
  assert1(cmsg -> id < q -> size);

  switch (cmsg -> event)
  {
    case VIRTIO_CONSOLE_DEVICE_ADD:
    {
      //request to add port with id cmsg -> id
      LOG("Recieved request to add port device #%d\n", cmsg -> id);
      u16 base_id = RN(cmsg -> id);
      virtio_set_queue(&cdev -> vdev, base_id, 
          virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, base_id)));
      virtio_set_queue(&cdev -> vdev, base_id + 1,
          virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, base_id + 1)));


      console_send_control_message(cdev, cmsg -> id, VIRTIO_CONSOLE_PORT_READY, 1);
    }break;

    case VIRTIO_CONSOLE_PORT_NAME:
    {
      LOG("Recieved port name from device for port #%d\n", cmsg -> id);
      u16 name_len = list -> len - sizeof(struct virtio_console_control);
      u8 name[name_len + 1];
      memcpy(buffer + sizeof(struct virtio_console_control), name,  name_len);
      name[name_len] = '\0';

      LOG("PORT_NAME = %s\n", name);
    }break;

    case VIRTIO_CONSOLE_PORT_OPEN:
    {
      LOG("Recieved request to open port #%d\n", cmsg -> id);
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
}

void
setup_rx(struct virtio_console *cdev, u32 vqid)
{
  struct virtio_queue *q = virtio_get_queue(&cdev -> vdev, vqid);
  u32 iobase = cdev -> vdev.iobase;
  virtio_queue_enqueue(q, kzmalloc(0x400), 400, VQ_IN);
  virtio_queue_kick(q, iobase);
}

void
console_init_port(struct virtio_console *cdev, struct virtio_port *port)
{
  u8 base_id = RN(port -> id);
  struct virtio_queue *rq0 = virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, base_id));
  struct virtio_queue *tq0 = virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, base_id + 1));

  virtio_set_queue(&cdev -> vdev, base_id, rq0);
  virtio_set_queue(&cdev -> vdev, base_id + 1, tq0);


  //TODO(gio): assigns single page for each buffer may cause problems.
  init_port_buffer(&port -> inbuf);
  init_port_buffer(&port -> outbuf); 

  port -> vq_in  = rq0;
  port -> vq_out = tq0;
}

struct virtio_port*
console_add_port(struct virtio_console *cdev, u16 id)
{
  struct virtio_port *res = (struct virtio_port*)kzmalloc(sizeof(struct virtio_port));
  res -> id = id;

  console_init_port(cdev, res);

  return res;
}

b8
console_setup(struct virtio_dev *vdev)
{
  struct virtio_console *cdev = vdev_to_cdev(vdev);

  virtio_read_config(vdev, sizeof(struct virtio_console_config), (u8*)&cdev -> cfg);
  
  struct virtio_queue *cr0 = virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, 2));
  cr0 -> handle_input = console_handle_control_message;
  virtio_set_queue(&cdev -> vdev, 2, 
    cr0);
  virtio_set_queue(&cdev -> vdev, 3, 
    virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, 3)));

  setup_rx(cdev, 2);

  console_send_control_message(cdev, 0xffffffff, VIRTIO_CONSOLE_DEVICE_READY, 1);
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
    for (i32 i = 0; i < 32; i++)
    {
      struct virtio_queue *q = vdev -> virtq[i];
      if (q)
      {
        while (q -> last_buffer_seen < q -> used -> idx)
        {
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
}
