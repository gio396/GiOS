#ifndef __DEVICE_BUS_H__
#define __DEVICE_BUS_H__

#include <common.h>
#include <list.h>

#include "device.h"

struct device_bus
{
  i8  *bus_name;

  struct dlist_root devices;
  struct dlist_root drivers;


  struct dlist_node self;
};

struct device_bus*
register_device_bus(i8 *bus_name);

b8
bus_register_driver(struct device_bus *bus, struct device_driver *driver);

b8
bus_register_device(struct device_bus *bus, struct device *dev);

#endif 