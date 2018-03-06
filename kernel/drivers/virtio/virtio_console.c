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

#define CONSOLE_PORT_IS_CONSOLE (1 << 0)
#define CONSOLE_PORT_OPEN       (1 << 1)

struct port_buffer
{
  u8 *base;
  u32 len;
  u32 used;
};

void
init_port_buffer(struct port_buffer *buffer)
{
  buffer -> base = kalloc();
  buffer -> len = kb(4);
}

struct console_port
{
  struct virtio *vdev;

  struct port_buffer inbuf;
  struct port_buffer outbuf;

  struct virtio_queue *vq_in, *vq_out;

  //utf-8 name for port
  u8 *name;
  u8 flags;
  u16 id;
  u16 vq_base;

  struct dlist_node node;
};

struct virtio_console
{
  struct virtio_dev vdev;
  struct virtio_console_config cfg;

  u16 num_ports;
  struct console_port cmsg_port;

  //all the ports in doubly linked list!
  //port 0 is always present!
  struct console_port ports;
};

void
vdev_console_write(struct virtio_console *cdev, u16 port, u8 *buffer, size_t len);
void
port_update_inbuf(struct console_port *port, u32 len);
struct console_port*
console_add_port(struct virtio_console *cdev, u16 id);
void
port_write_data(struct console_port *port, u8 *data, u32 len);
struct console_port*
find_port_by_id(struct virtio_console *cdev, u16 id);

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
  struct console_port *cport = &cdev -> cmsg_port; 
  struct virtio_console_control ctrl = {};

  ctrl.id = id;
  ctrl.event = event;
  ctrl.value = value;

  port_write_data(cport, (u8*)&ctrl, sizeof(ctrl));
}

void
console_handle_control_message(struct virtio_console *cdev, struct scatter_list list)
{
  u8 *buffer = list.buffer;
  struct virtio_console_control *cmsg = (struct virtio_console_control*)buffer;
  struct console_port *cport = &cdev -> cmsg_port;
  struct virtio_queue *q = cport -> vq_in;
  assert1(cmsg -> id < q -> size);

  port_update_inbuf(cport, list.len);

  switch (cmsg -> event)
  {
    case VIRTIO_CONSOLE_DEVICE_ADD:
    {
      //request to add port with id cmsg -> id
      LOG("Recieved request to add port device #%d\n", cmsg -> id);
      console_add_port(cdev, cmsg -> id);

      console_send_control_message(cdev, cmsg -> id, VIRTIO_CONSOLE_PORT_READY, 1);
    }break;

    case VIRTIO_CONSOLE_PORT_NAME:
    {
      struct console_port *port = find_port_by_id(cdev, cmsg -> id);

      if (!port)
      {
        assert1(0);
      }

      LOG("Recieved port name from device for port #%d\n", cmsg -> id);
      u16 name_len = list.len - sizeof(struct virtio_console_control);
      u8 *name = (u8*)kzmalloc(name_len);
      memcpy(buffer + sizeof(struct virtio_console_control), name,  name_len);
      name[name_len] = '\0';

      port -> name = name;
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
port_add_inbuf(struct console_port *port)
{
  struct virtio_queue *vq = port -> vq_in;
  struct port_buffer *buf = &port -> inbuf;

  u8 *buffer_base = buf -> base + buf -> used;
  u32 len = buf -> len - buf -> used;

  virtio_queue_enqueue(vq, buffer_base, len, VQ_IN);
  virtio_queue_kick(vq);
}

void
port_update_inbuf(struct console_port *port, u32 len)
{
  struct port_buffer *buf = &port -> inbuf;

  if (buf -> used + len > buf -> len - 8)
  {
    buf -> used = 0;
  }
  else
  {
    buf -> used += len;
  }

  port_add_inbuf(port);
}

u8 *port_get_outbuff(struct console_port *port, u32 len)
{
  struct port_buffer *buf = &port -> outbuf;
  assert1(len < buf -> len);

  if (buf -> used + len > buf -> len)
  {
    buf -> used = 0;
  }
  
  u8 *res = buf -> base + buf -> used; 
  buf -> used += len;

  return res;
}

void
port_write_data(struct console_port *port, u8 *data, u32 len)
{
  struct virtio_queue *vq = port -> vq_out;
  u8 *ring_buffer = port_get_outbuff(port, len);
  memcpy(data, ring_buffer, len);

  virtio_queue_enqueue(vq, ring_buffer, len, VQ_OUT);
  virtio_queue_kick(vq);
}

struct console_port*
find_port_by_id(struct virtio_console *cdev, u16 id)
{
  struct console_port *ports = &cdev -> ports;
  struct dlist_node *it;

  FOR_EACH_LIST(it, &ports -> node)
  {
    struct console_port *port = CONTAINER_OF(it, struct console_port, node);
    if (port -> id == id)
    {
      return port;
    }
  }

  return NULL;
}

void
console_init_port(struct virtio_console *cdev, struct console_port *port)
{
  u8 base_id = port -> vq_base;
  struct virtio_queue *rq0 = virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, base_id));
  struct virtio_queue *tq0 = virtio_create_queue(virtio_get_queue_size(&cdev -> vdev, base_id + 1));

  virtio_set_queue(&cdev -> vdev, base_id, rq0);
  virtio_set_queue(&cdev -> vdev, base_id + 1, tq0);

  //TODO(gio): assigns single page for each buffer may cause problems.
  init_port_buffer(&port -> inbuf);
  init_port_buffer(&port -> outbuf); 

  port -> vq_in  = rq0;
  port -> vq_out = tq0;

  port_add_inbuf(port);
}

struct console_port*
console_add_port(struct virtio_console *cdev, u16 id)
{
  struct console_port *res = (struct console_port*)kzmalloc(sizeof(struct console_port));
  res -> id = id;
  res -> vq_base = RN(id);

  console_init_port(cdev, res);

  struct dlist_root root = {&cdev -> ports.node};
  dlist_insert_tail(&root, &res -> node);
  return res;
}

i8
port_has_unseen_buffers(struct console_port *port)
{
  struct virtio_queue *vq = port -> vq_in;

  return virtio_queue_has_unseen_buffers(vq);
}

struct scatter_list
port_get_in_head(struct console_port *port)
{
  struct virtio_queue *vq = port -> vq_in;
  assert1(virtio_queue_has_unseen_buffers(vq));

  return virtio_queue_dequeue(vq);
}

b8
console_setup(struct virtio_dev *vdev)
{
  struct virtio_console *cdev = vdev_to_cdev(vdev);

  virtio_read_config(vdev, sizeof(struct virtio_console_config), (u8*)&cdev -> cfg);

  cdev -> cmsg_port.vq_base = 2;
  console_init_port(cdev, &cdev -> cmsg_port);

  cdev -> cmsg_port.vq_base = 0;
  console_init_port(cdev, &cdev -> ports);

  console_send_control_message(cdev, 0xffffffff, VIRTIO_CONSOLE_DEVICE_READY, 1);

  new_timer(5*1000 *1000, write_later, (u32)cdev);
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
    //check cmsg port first
    struct console_port *cport = &cdev -> cmsg_port;
    while(port_has_unseen_buffers(cport))
    {
      console_handle_control_message(cdev, port_get_in_head(cport));
    }

    //now check data ports
    struct dlist_node *it;
    FOR_EACH_LIST(it, &cdev -> ports.node)
    {
      struct console_port *port = CONTAINER_OF(it, struct console_port, node);
      while(port_has_unseen_buffers(port))
      {
        //TODO(gio): handle input
      }
    }
  }
  if ((isr & 0x3) == 0x3)
  {
    LOG("CONFIGURATION CHANGE!\n");
    //TODO(gio): handle configuration change
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
vdev_console_write(struct virtio_console *cdev, u16 id, u8 *buffer, size_t len)
{
  struct console_port *port = find_port_by_id(cdev, id);

  if (port == NULL)
  {
    LOG("NULL!\n");
    return;
  }

  port_write_data(port, buffer, len);
}
