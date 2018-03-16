#ifndef __BLOCK_DEVICE__
#define __BLOCK_DEVICE__

#include <drivers/device/device.h>

typedef void(blk_rq_done_fn)(struct request, u32);
typedef void(rq_make_blk_rq)(struct request_queue*, struct request*);

#define BLK_RQ_T_IN    0x0
#define BLK_RQ_T_OUT   0x1
#define BLK_RQ_T_FLUSH 0x2

#define BLK_RQ_R_OK    0x0
#define BLK_RQ_R_IOERR 0x1
#define BLK_RQ_R_UNSUP 0x2

struct request_queue
{
  struct block_device *bdev;
  slist_root ql;

  struct request *head;
  struct request *tail;
  u32 len;
};

struct blk_meta_rq_dt
{
  struct request_queue *rq;

  u64  qr_extra_cmd_size
  void *dri_data;
  u64  queue_depth;
  u64  hw_queues;;

  blk_mt_rq_ops *ops;
};

struct request
{
  u32 type;
  struct scatterist *sl;
  struct request_queue *rq;

  blk_rq_done_fn *rq_dn_cb;
  void           *rq_dn_data;;

  struct 
  {
    u64 data_len;
    u64 sector;
  } priv;

  slist_node self;
};

struct blk_mt_rq_ops
{
  i32 init_request(struct blk_meta_rq_dt*, struct request*);
  i32 make_request(struct blk_meta_rq_dt*, struct request*);
  void request_done(struct blk_meta_rq_dt*, struct request*);
};

struct block_device
{
  struct device dev;
  struct blk_meta_rq_dt *rq_data;

  struct block_driver *driver;

  dlist_node *next;
};

struct block_driver
{
  struct device_driver dev_dri;
};

#endif
