#include "device_bus.h"

#include <memory.h>

global struct dlist_root busses;

struct device_bus*
register_device_bus(i8 *bus_name)
{
  struct device_bus *res = (struct device_bus*)kzmalloc(sizeof(struct device_bus));
  res -> bus_name = bus_name;

  dlist_insert_tail(&busses, &res -> self);

  return res;
}

b8
bus_register_driver(struct device_bus *bus, struct device_driver *driver)
{
  struct dlist_root *drivers = &bus -> drivers;
  dlist_insert_tail(drivers, &driver -> self);

  return 1;
}

b8
bus_register_device(struct device_bus *bus, struct device *dev)
{
  struct dlist_root *devices = &bus -> devices;
  dlist_insert_tail(devices, &dev -> self);

  return 1;
}