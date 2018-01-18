#include "virtio_console.h"

#include "arch/x86/framebuffer.h"

#include <macros.h>
#include <list.h>

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
  struct virtio_console *cdev = &console_stack[console_head++];
  cdev -> vdev = *vdev;

  return cdev;
}

b8
init_vdev_console(struct virtio_dev *vdev)
{
  LOG("Initializing virtio console(console)\n");
  struct virtio_console *cdev = new_console(vdev);

  u32 iobase = cdev -> vdev.iobase;
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
    LOG("Features nagotiation failed!");
    virtio_add_status(iobase, VIRTIO_STATUS_FAILED);
    return 0;
  }

  virtio_add_status(iobase, VIRTIO_STATUS_DRI_OK);

  virtio_set_queue(&cdev -> vdev, 0, 
    virtio_create_queue("tx",  virtio_get_queue_size(&cdev -> vdev, 0)));
  virtio_set_queue(&cdev -> vdev, 1, 
    virtio_create_queue("rx",  virtio_get_queue_size(&cdev -> vdev, 1)));
  virtio_set_queue(&cdev -> vdev, 2, 
    virtio_create_queue("ctx", virtio_get_queue_size(&cdev -> vdev, 2)));
  virtio_set_queue(&cdev -> vdev, 3, 
    virtio_create_queue("crx", virtio_get_queue_size(&cdev -> vdev, 3)));

  return 1;
}