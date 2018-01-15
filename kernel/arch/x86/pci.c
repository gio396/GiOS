#include "pci.h"
#include "io.h"
#include "timer.h"
#include "framebuffer.h"
#include "page.h"

#include <macros.h>
#include <string.h>

//Configuration Space Access Mechanism #1 adress.
// 31          30 - 24   23 - 16     15 - 11         10 - 8            7 - 2            1-0
// Enable Bit  Reserved  Bus Number  Device Number   Function Number   Register Number  00

#define PCI_VENDOR_OFFSET      0x00
#define PCI_DEVICE_ID_OFFSET   0x02
#define PCI_CLASS_OFFSET       0x0a
#define PCI_BASE_CLASS_OFFSET  0x0b
#define PCI_HEADER_TYPE_OFFSET 0x0d

#define PCI_01H_SECONDARY_BUS_OFFSET 0x18

#define PCI_GET_BASE_CLASS(bc) ((bc >> 8) & 0x00ff)
#define PCI_GET_SUB_CLASS(bc)  ((bc) & 0x00ff)

struct linear_allocator
{
  void *mem_base;
  u32   mem_used;
} la;

void*
allocate(struct linear_allocator *la, u32 size)
{
  if (la -> mem_used + size > kb(4))
    return NULL;

  void *res = la -> mem_base + la -> mem_used;
  la -> mem_used += size;

  return res;
}

struct pci_bus*
pci_append_bus(struct pci_bus *parrent)
{
  struct pci_bus *res = (struct pci_bus*)allocate(&la, sizeof(struct pci_bus));
  ZERO_STRUCT(res, struct pci_bus);

  res -> parrent = parrent;

  dlist_insert_tail(&parrent -> children, &res -> next);

  return res;
}

struct pci_dev*
pci_append_device(struct pci_bus *bus)
{
  struct pci_dev *res = (struct pci_dev*)allocate(&la, sizeof(struct pci_dev));
  ZERO_STRUCT(res, struct pci_dev);

  dlist_insert_tail(&bus -> devices, &res -> self);

  return res;
}

void
check_function(u8 bus, u8 device, u8 function, struct pci_bus *lbus, struct pci_dev *ldev);

u16
pci_config_read_word(u8 bus, u8 slot, u8 func, u8 offset)
{
  u32 adress;
  u32 lbus  = bus;
  u32 lslot = slot;
  u32 lfunc = func;
  u32 tmp   = 0;

  adress = ((u32)0x80000000 | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc));

  outl(0xCF8, adress);
  tmp = (u16)((inl(0xCFC) >> ((offset & 2) *8)) & 0xffff);

  return tmp;
}

void
check_device(u8 bus, u8 device, struct pci_bus *lbus)
{
  u8 function = 0;
  u16 header_type;
  u16 vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_OFFSET);

  if (vendor_id == 0xffff) return; //non-existant device

  struct pci_dev *ldevice = pci_append_device(lbus);

  check_function(bus, device, function, lbus, ldevice);
  header_type = pci_config_read_word(bus, device, function, PCI_HEADER_TYPE_OFFSET);
  if ((header_type & 0x80) != 0)
  {
    //multifunction device!
    for (function = 1; function < 8; function++)
    {
      if (pci_config_read_word(bus, device, function, PCI_VENDOR_OFFSET) != 0x80)
      {
        LOG("Additional functions for single pci device FOUND!! WARNING NOT SUPPORTED");
        halt();

        //TODO(gio): add  support for devices with multiple functions!
        check_function(bus, device, function, lbus, ldevice);
      }
    }
  }
}

void
walk_bus(u8 bus, struct pci_bus *self)
{
  self -> bus = bus;

  for (i32 d = 0; d < 32; d++)
  {
    LOG("Cheking device(0x%x)\n", d);
    check_device(bus, d, self);
  }
}

void
check_function(u8 bus, u8 device, u8 function, struct pci_bus *lbus, struct pci_dev *ldev)
{
  u8 base_class;
  u8 sub_class;
  u8 sbus;
  u16 lclass = pci_config_read_word(bus, device, function, PCI_CLASS_OFFSET);
  base_class = PCI_GET_BASE_CLASS(lclass);
  sub_class = PCI_GET_SUB_CLASS(lclass);

  LOG("class base 0x%0x\nclass sub 0x%0x\n", base_class, sub_class);

  ldev -> base_class = base_class;
  ldev -> sub_class = sub_class;

  if ((base_class == 0x60) && (sub_class == 0x04))
  {
    sbus = (pci_config_read_word(bus, device, function, PCI_01H_SECONDARY_BUS_OFFSET) >> 8) && 0x00ff;

    struct pci_bus *nbus = pci_append_bus(lbus);
    nbus -> self = ldev;
    
    walk_bus(sbus, nbus);
  }
}

void
pci_init_bus_mem()
{
  la.mem_base = kalloc();
  la.mem_used = 0;

  global_bus = (struct pci_bus*)allocate(&la, sizeof(struct pci_bus));
  ZERO_STRUCT(global_bus, struct pci_bus);
}

void
print_pci(struct pci_bus *bus, i32 append)
{
  struct dlist_node *head = bus -> devices.dlist_node;
  struct dlist_node *it = NULL;

  for (i32 i = 0; i < append; i++)
    LOG(" ");

  LOG("bus 0x%02x: ", bus -> bus);

  FOR_EACH_LIST(it, head)
  {
    struct pci_dev *dev = CONTAINER_OF(it, struct pci_dev, self);
    LOG("->(0x%08x) 0x%02x%02x ", dev, dev -> base_class, dev -> sub_class);
  }

  LOG("\n");

  head = bus -> children.dlist_node;

  FOR_EACH_LIST(it, head)
  {
    struct pci_bus *child = CONTAINER_OF(it, struct pci_bus, next);
    print_pci(child, append + 1);
  }
}

void
pci_init_enum()
{
  pci_init_bus_mem();
  u8 function = 0;
  u8 bus;

  u16 header_type = pci_config_read_word(0, 0, 0, PCI_HEADER_TYPE_OFFSET);

  if ((header_type & 0x80) == 0)
  {
    LOG("SINGLE pci host controller!\n");
    walk_bus(0, global_bus);
  }
  else
  {
    for (function = 0; function < 8; function++)
    {
      u16 vendor_id = pci_config_read_word(0, 0, function, PCI_VENDOR_OFFSET);
      if (vendor_id != 0xffff) break;

      bus = function;
      walk_bus(bus, global_bus);
    }
  }

  print_pci(global_bus, 0);
}