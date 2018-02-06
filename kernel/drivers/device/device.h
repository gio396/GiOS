#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <common.h>
#include <list.h>

struct device
{
  i8 *name;

  struct device_bus *bus;

  struct device_driver *driver;
  struct dlist_node self;
};

struct device_driver
{
  struct device_bus *bus;

  void (*enable)(struct device *dev);
  void (*disable)(struct device *dev);

  struct dlist_node self;
};

#endif