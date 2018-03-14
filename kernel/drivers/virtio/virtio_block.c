
#include "virtio_block.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/idt.h>
#include <arch/x86/page.h>

#include <drivers/pci/pci.h>
#include <drivers/virtio/virtio_queue.h>

#include <macros.h>
#include <list.h>
#include <string.h>
#include <timer.h>
#include <memory.h>

#define VIRTIO_BLK_F_SIZE_MAX   1
#define VIRTIO_BLK_F_SEG_MAX    2
#define VIRTIO_BLK_F_GEOMETRY   4
#define VIRTIO_BLK_F_RO         5
#define VIRTIO_BLK_F_BLK_SIZE   6
#define VIRTIO_BLK_F_FLUSH      9
#define VIRTIO_BLK_F_TOPOLOGY   10
#define VIRTIO_BLK_F_CONFIG_WCE 11
#define VIRTIO_BLK_F_BARRIER    0
#define VIRTIO_BLK_F_SCSI       7

#define VIRTIO_BLK_T_IN         0
#define VIRTIO_BLK_T_OUT        1
#define VIRTIO_BLK_T_FLUSH      4  

#define VIRTIO_BLK_S_OK         0
#define VIRTIO_BLK_S_IOERR      1
#define VIRTIO_BKL_S_UNSUPP     2

#define REQUEST_BUFFER_SIZE     32

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

struct virtio_block_req_out_hdr
{
  u32 type;
  u32 reserved;
  u64 sector;
};

struct virtio_block_req_block
{
  u8 block[512];
};

struct virtio_block_req_in_hdr
{
  u8 status;
};

struct block_request_port
{
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
block_request_port_init(struct virtio_block *bdev)
{
  struct block_request_port *rport = &bdev -> rport;
   struct virtio_queue *reqq = virtio_create_queue(virtio_get_queue_size(&bdev -> vdev, 0));

  virtio_set_queue(&bdev -> vdev, 0, reqq);
  rport -> reqq = reqq;
}

void
vdev_read_request(struct virtio_block *vdev, u32 sector, u8 *buffer, size_t len)
{
  struct block_request_port *rport = &vdev -> rport;
  u8 *hdrs = kzmalloc(sizeof(struct virtio_block_req_in_hdr) + sizeof(struct virtio_block_req_out_hdr));

  struct virtio_block_req_out_hdr *ohdr = (struct virtio_block_req_out_hdr*)hdrs;
  struct virtio_block_req_in_hdr  *ihdr = (struct virtio_block_req_in_hdr*)(ohdr + 1);

  u32 sector_count = (len + 511) / 512;
  u32 sl_size = sector_count + 2;

  ohdr -> type = VIRTIO_BLK_T_IN;
  ohdr -> sector = sector;

  LOGV("%p", ohdr);
  LOGV("%p", ihdr);
  LOGV("%p", buffer);

  struct virtio_queue *reqq = rport -> reqq;

  struct scatterlist sl[sl_size];
  sl_list_init(sl, sl_size);

  sl_bind_buffer(&sl[0], ohdr, sizeof(struct virtio_block_req_out_hdr));
  sl_bind_attribute(&sl[0], SL_USER_0, VQ_OUT);

  for (u32 i = 1; i < sl_size - 1; i++)
  {  
    sl_bind_buffer(&sl[i], buffer + 512 * (i - 1),  sizeof(struct virtio_block_req_block));
    sl_bind_attribute(&sl[i], SL_USER_0, VQ_IN);
  }

  sl_bind_buffer(&sl[sl_size - 1], ihdr, sizeof(struct virtio_block_req_in_hdr));
  sl_bind_attribute(&sl[sl_size - 1], SL_USER_0, VQ_IN);

  virtio_queue_enqueue(reqq, sl);
  virtio_queue_kick(reqq);
}

void
vdev_write_request(struct virtio_block *vdev, u32 sector, u8 *buffer, size_t len)
{
  struct block_request_port *rport = &vdev -> rport;
  u8 *hdrs = kzmalloc(sizeof(struct virtio_block_req_in_hdr) + sizeof(struct virtio_block_req_out_hdr));

  struct virtio_block_req_out_hdr *ohdr = (struct virtio_block_req_out_hdr*)hdrs;
  struct virtio_block_req_in_hdr  *ihdr = (struct virtio_block_req_in_hdr*)(ohdr + 1);

  u32 sector_count = (len + 511) / 512;
  u32 sl_size = sector_count + 2;

  ohdr -> type = VIRTIO_BLK_T_OUT;
  ohdr -> sector = sector;

  struct virtio_queue *reqq = rport -> reqq;

  struct scatterlist sl[sl_size];
  sl_list_init(sl, sl_size);

  sl_bind_buffer(&sl[0], ohdr, sizeof(struct virtio_block_req_out_hdr));
  sl_bind_attribute(&sl[0], SL_USER_0, VQ_OUT);

  for (u32 i = 1; i < sl_size - 1; i++)
  {  
    sl_bind_buffer(&sl[i], buffer + 512 * (i - 1),  sizeof(struct virtio_block_req_block));
    sl_bind_attribute(&sl[i], SL_USER_0, VQ_IN);
  }

  sl_bind_buffer(&sl[sl_size - 1], ihdr, sizeof(struct virtio_block_req_in_hdr));
  sl_bind_attribute(&sl[sl_size - 1], SL_USER_0, VQ_IN);

  virtio_queue_enqueue(reqq, sl);
  virtio_queue_kick(reqq);
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

u8 *buffer;

internal void
write_later(u32 addr)
{
  struct virtio_block *bdev = (struct virtio_block*)(addr);
  buffer = kzmalloc(512);
  vdev_read_request(bdev,  1, buffer, 512);
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

  new_timer(2*1000*1000, write_later, (u32)bdev);

  return 1;
}

void
bdev_interrupt(const union biosregs *iregs, struct virtio_dev *vdev)
{
  struct virtio_block *bdev =  vdev_to_bdev(vdev);
  u8 isr = virtio_get_isr(vdev);

  if ((isr & 0x1) == 0x1)
  {
    struct block_request_port *rport = &bdev -> rport;
    if (virtio_queue_has_unseen_buffers(rport -> reqq))
    {
      struct scatterlist *sl = virtio_queue_dequeue(rport -> reqq);
      u8 *buffer = sl_get_buffer(sl);

      LOGV("%p", buffer);
      struct virtio_block_req_out_hdr *hdr = (struct virtio_block_req_out_hdr*)buffer;
      LOGV("%p", hdr -> type);  
    }
  }
  if ((isr & 0x3) == 0x3)
  {
    LOG("CONFIGURATION_CHANGE!\n");
  }

  LOG("INTERRUPT!\n");
  LOGV("%d", buffer[0]);
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