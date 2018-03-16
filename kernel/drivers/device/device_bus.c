#include "device_bus.h"

#include <memory.h>

global struct dlist_root busses;

struct device_bus_rq
{
  i8  *name;
  u64 req_ext_data;
};

struct device_bus*
find_bus_by_name(i8 *name)
{
  dlist_node *it;
  FOR_EACH_LIST(it, busses -> head)
  {
    struct device_bus *bus = CONTAINER_OF(it, struct device_bus, self);
    if (strcmp(self -> name, name) == 0)
    {
      return bus;
    }
  }

  return NULL;
}

struct device_bus*
register_device_bus(struct device_bus_rq *rq)
{
  if (find_bus_by_name(rq -> name) != NULL)
  {
    return NULL;
  }

  struct device_bus *res = (struct device_bus*)kzmalloc(sizeof(struct device_bus) rq -> req_ext_data);
  res -> bus_name = rq -> name;

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