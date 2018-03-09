
#include "virtio_block.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/idt.h>
#include <arch/x86/page.h>

#include <drivers/pci/pci.h>

#include <macros.h>
#include <list.h>
#include <string.h>
#include <memory.h>

#define VIRTIO_BLK_F_SIZE_MAX   (1)
#define VIRTIO_BLK_F_SEG_MAX    (2)
#define VIRTIO_BLK_F_GEOMETRY   (4)
#define VIRTIO_BLK_F_RO         (5)
#define VIRTIO_BLK_F_BLK_SIZE   (6)
#define VIRTIO_BLK_F_FLUSH      (9)
#define VIRTIO_BLK_F_TOPOLOGY   (10)
#define VIRTIO_BLK_F_CONFIG_WCE (11)
#define VIRTIO_BLK_F_BARRIER    (0)
#define VIRTIO_BLK_F_SCSI       (7)

struct virtio_block_config
{
  u64 capacity;
  u32 size_max;
  u32 seg_max;

  struct viortio_block_geometry
  {
    u16 cylinders;
    u8  heads;
    u8  sectors;
  } geometry;

  u32 block_size;

  struct virtio_block_topology
  {
    u8  physical_block_exp;
    u8  alignment_offset;
    u16 min_io_size;
    u32 opt_io_size;
  } topology;
  u8 writeback;
};

struct virtio_block_req
{
  u32 type;
  i32 reserved;
  u64 sector;
  u8 data[512];
  u8 status;
};

struct virtio_block_req_buffer
{
  u32 head;
  u32 tail;
  //TODO(gio): change to 16 malloc because malloc sucks right now :()
  struct virtio_block_req request_buffer[3];
};

struct block_request_port
{
  struct virtio_block_req_buffer buffer;
  struct virtio_queue *reqq;
};

struct virtio_block
{
  struct virtio_dev vdev;
  struct virtio_block_config cfg;
  
  struct block_request_port rport;
};

struct virtio_block*
new_block_device(struct pci_dev *dev)
{
  struct virtio_block *bdev = (struct virtio_block *)kzmalloc(sizeof(struct virtio_block));

  return bdev;
}

struct virtio_block*
vdev_to_bdev(struct virtio_dev *dev)
{
  return CONTAINER_OF(dev, struct virtio_block, vdev);
}

void
port_buffer_init(struct block_request_port *rport)
{
  struct virtio_block_req_buffer *buffer = &rport -> buffer;
  buffer -> head = buffer -> tail = 0;
}

void
block_request_port_init(struct virtio_block *bdev)
{
  struct block_request_port *rport = &bdev -> rport;
  port_buffer_init(rport);

 struct virtio_queue *reqq = virtio_create_queue(virtio_get_queue_size(&bdev -> vdev, 0));
  virtio_set_queue(&bdev -> vdev, 0, reqq);

 rport -> reqq = reqq;;
}

#define CHECK_F(fin, feat)                                    \
    do{                                                       \
        b32 __avail = (((fin) & (1 << (feat))) != 0);         \
        LOG(#feat " is%savailable\n", __avail ? " ":" NOT "); \
    }while(0)                                                 \

b8
bdev_features(struct virtio_dev *vdev, u32 features)
{
  LOG("Virtio Block got features %016b\n", features);
  CHECK_F(features, VIRTIO_BLK_F_SIZE_MAX);
  CHECK_F(features, VIRTIO_BLK_F_SEG_MAX);
  CHECK_F(features, VIRTIO_BLK_F_GEOMETRY);
  CHECK_F(features, VIRTIO_BLK_F_RO);
  CHECK_F(features, VIRTIO_BLK_F_BLK_SIZE);
  CHECK_F(features, VIRTIO_BLK_F_FLUSH);
  CHECK_F(features, VIRTIO_BLK_F_TOPOLOGY);
  CHECK_F(features, VIRTIO_BLK_F_CONFIG_WCE);
  CHECK_F(features, VIRTIO_BLK_F_BARRIER);
  CHECK_F(features, VIRTIO_BLK_F_SCSI);

  u32 mandatory_features = (1 << VIRTIO_BLK_F_BLK_SIZE);

  return vdev_confirm_features(vdev, mandatory_features);
}

b8
bdev_setup(struct virtio_dev *vdev)
{
  struct virtio_block *bdev = vdev_to_bdev(vdev);

  virtio_read_config(vdev, sizeof(struct virtio_block_config), (u8*)&bdev -> cfg);
  struct virtio_block_config *cfg = &bdev -> cfg;

  LOGV("%u", cfg -> capacity);
  LOGV("%d", cfg -> size_max);
  LOGV("%d", cfg -> seg_max);
  LOG("Geometry\n");
  LOGV("%d", cfg -> geometry.cylinders);
  LOGV("%d", cfg -> geometry.heads);
  LOGV("%d", cfg -> geometry.sectors);
  LOGV("%u", cfg -> block_size);
  LOG("Topology\n");
  LOGV("%d", cfg -> topology.physical_block_exp);
  LOGV("%d", cfg -> topology.alignment_offset);
  LOGV("%d", cfg -> topology.min_io_size);
  LOGV("%d", cfg -> topology.opt_io_size);
  LOGV("%d", cfg -> writeback);

  block_request_port_init(bdev);

  return 1;
}

void
bdev_interrupt(const union biosregs *iregs, struct virtio_dev *vdev)
{
}

struct virtio_driver virtio_block_driver = {
  .probe_features = bdev_features,
  .setup = bdev_setup,
  .ievent = bdev_interrupt,
};

struct virtio_dev*
init_vdev_block(struct pci_dev *dev)
{
  struct virtio_block *bdev = new_block_device(dev);

  bdev -> vdev.driver = &virtio_block_driver;

  return &bdev -> vdev;
}