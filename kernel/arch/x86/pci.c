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

u8
pci_config_read_byte(u8 bus, u8 slot, u8 func, u8 offset)
{
  u32 adress;
  u32 lbus  = bus;
  u32 lslot = slot;
  u32 lfunc = func;
  u32 tmp   = 0;

  adress = ((u32)0x80000000 | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc));

  outl(0xCF8, adress);
  tmp = (u16)((inl(0xCFC) >> ((offset % 4) * 4)) & 0xff);

  return tmp;
}

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

u32
pci_config_read_dword(u8 bus, u8 slot, u8 func, u8 offset)
{
  u32 adress;
  u32 lbus  = bus;
  u32 lslot = slot;
  u32 lfunc = func;
  u32 tmp   = 0;

  adress = ((u32)0x80000000 | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc));

  outl(0xCF8, adress);
  tmp = inl(0xCFC);

  return tmp;
}

void
pci_config_write_dword(u8 bus, u8 slot, u8 func, u8 offset, u32 value)
{
  u32 adress;
  u32 lbus  = bus;
  u32 lslot = slot;
  u32 lfunc = func;

  adress = ((u32)0x80000000 | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc));

  outl(0xCF8, adress);
  outl(0xCFC, value);
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
    // LOG("Cheking device(0x%x)\n", d);
    check_device(bus, d, self);
  }
}

u32
pci_size(u32 base, u32 mask)
{
  u32 size = base & mask;

  size &= ~(size - 1);

  return size;
}

b32
pci_dev_check_msix_capability(struct pci_dev *dev)
{
  u32 pos = pci_dev_find_capability(dev, PCI_MSIX_CAP);

  LOG("MSI POS = %p\n", pos);

  if (pos == 0)
  {
    dev -> msix = 0;
    return 0;
  }

  //TODO(gio): setup msi vectors
  dev -> msix = 1;
  return 1;
}

void
check_function(u8 bus, u8 device, u8 function, struct pci_bus *lbus, struct pci_dev *ldev)
{
  u16 vendor_id;
  u16 device_id;
  u16 subsystem_id;
  u8 base_class;
  u8 sub_class;
  u8 sbus;
  u8 pcapabilites;

  u16 lclass = pci_config_read_word(bus, device, function, PCI_CLASS_OFFSET);
  base_class = PCI_GET_BASE_CLASS(lclass);
  sub_class = PCI_GET_SUB_CLASS(lclass);
  vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_OFFSET);
  device_id = pci_config_read_word(bus, device, function, PCI_DEVICE_ID_OFFSET);
  subsystem_id = pci_config_read_word(bus, device, function, PCI_SUBSYSTEM_ID_OFFSET);
  pcapabilites = pci_config_read_word(bus, device, function, PCI_CAPABILITIES_POINTER_OFFSET) & 0x00ff;

  LOG("class base 0x%0x\nclass sub 0x%0x\n", base_class, sub_class);

  ldev -> bus          = lbus;
  ldev -> device       = device;
  ldev -> func         = function;
  ldev -> vendor_id    = vendor_id;
  ldev -> device_id    = device_id;
  ldev -> base_class   = base_class;
  ldev -> sub_class    = sub_class;
  ldev -> subsystem_id = subsystem_id;
  ldev -> capabilities = pcapabilites;

  // fill resources!
  for (i32 i = 0; i < 6; i++)
  {
    ldev -> BAR[i] = pci_config_read_dword(bus, device, function, PCI_BASE_ADRESS_OFFSET(i));
    LOG("BAR[%d] = 0x%08x.\n", i, ldev -> BAR[i]);
    u32 bar = ldev -> BAR[i];

    if (!bar) continue;

    pci_config_write_dword(bus, device, function, bar, 0xffffffff);
    u32 len = pci_config_read_dword(bus, device, function, bar);

    if (bar & 1)
    {
      u32 base_addr = bar & BASE_ADDRESS_IO_MASK;
      u32 pci_end = base_addr + pci_size(len, BASE_ADDRESS_IO_MASK & 0xffff);

      ldev -> resources[i].type = RESOURCE_IO;
      ldev -> resources[i].start = base_addr;
      ldev -> resources[i].end = pci_end;
    }
    else
    {
      u32 base_addr = bar & BASE_ADDRESS_MEM_MASK;
      u32 pci_end = base_addr + pci_size(len, BASE_ADDRESS_MEM_MASK);

      LOG("Actual mem base %p!\n", base_addr);

      ldev -> resources[i].type = RESOURCE_MEM;
      ldev -> resources[i].start = base_addr;
      ldev -> resources[i].end = pci_end;
    }
  }

  if (pci_dev_check_msix_capability(ldev) == 0)
  {

  }

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
    LOG("->(0x%08x venodor:0x%4x device:0x%4x) 0x%02x%02x subsystem 0x%04x:", dev, dev -> vendor_id, dev -> device_id, dev -> base_class, dev -> sub_class, dev -> subsystem_id);
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

struct dlist_node*
pci_lookup_device_start()
{
  return global_bus -> devices.dlist_node;
}

struct dlist_node*
pci_lookup_device_next(struct dlist_node *it, u16 vendor_id)
{
  FOR_EACH_LIST_C(it)
  {
    struct pci_dev *dev = CONTAINER_OF(it, struct pci_dev, self);
    if (dev -> vendor_id == vendor_id)
    {
      return it;
    }
  }

  //TODO(gio): Check other busses!\n

  return NULL;
}

struct dlist_node*
pci_lookup_device(struct dlist_node *it, u16 vendor_id)
{
  it = pci_lookup_device_start();
  return pci_lookup_device_next(it, vendor_id);
}

void
pci_dev_read_config_byte(struct pci_dev *dev, u8 offset, u8 *res)
{
  u8 bus  = dev -> bus -> bus;
  u8 device = dev -> device;
  u8 func = dev -> func;
  u16 tmp;

  tmp = pci_config_read_word(bus, device, func, offset);

  LOG("pci_dev_read_config_byte 0x%04x\n", tmp);
  *res = (u8)(tmp & 0xff);
}

void
pci_dev_read_config_word(struct pci_dev *dev, u8 offset, u16 *res)
{
  u8 bus  = dev -> bus -> bus;
  u8 device = dev -> device;
  u8 func = dev -> func;
  u16 tmp;

  tmp = pci_config_read_word(bus, device, func, offset);

  *res = tmp;
}

i32
pci_find_cap_start(u8 bus, u8 device, u8 function)
{
  u16 status;
  status = pci_config_read_word(bus, device, function, PCI_STATUS_OFFSET);

  //status bit 4 must be set for capabilities pointer.
  if ((status & 0x10) == 0)
    return 0;

  //TODO(gio): pci_cardbus capabilities offset is different!
  return PCI_CAPABILITIES_POINTER_OFFSET;
}

i32
pci_find_cap_next_ttl(u8 bus, u8 device, u8 function, i32 pos, u8 cap, i32 *ttl)
{
  u8 id;
  u16 ent;

  LOG("posin = 0x%02x\n", pos);
  pos = pci_config_read_byte(bus, device, function, pos);
  LOG("posin = 0x%02x\n", pos);

  while((*ttl)--)
  {
    if (pos < 0x40)
      break;
    //least sagnificant 3 bits are reserved and should be ignored.s
    pos &= ~3;
    ent = pci_config_read_word(bus, device, function, pos);

    id = ent & 0xff;
    if (id == 0xff)
      break;

    LOG("capabilities ID = 0x%x\n", id);
    if (id == cap)
      return pos;

    pos = (ent >> 8);
  }

  return 0;
}

#define PCI_FIND_CAP_TTL 48

i32
pci_find_cap_next(u8 bus, u8 device, u8 function, i32 pos, u8 cap)
{
  i32 ttl = PCI_FIND_CAP_TTL;

  return pci_find_cap_next_ttl(bus, device, function, pos, cap, &ttl);
}

i32
pci_dev_find_capability(struct pci_dev *dev, u8 cap)
{
  i32 pos;

  u8 bus = dev -> bus -> bus;
  u8 device = dev -> device;
  u8 function = dev -> func;

  pos = pci_find_cap_start(bus, device, function);

  if (pos)
    pos = pci_find_cap_next(bus, device, function, pos, cap);

  return pos;
}

u32
pci_dev_get_iobase(struct pci_dev *dev)
{
  for (i32 i = 0; i < 6; i++)
  {
    struct pci_dev_resource *res = &dev -> resources[i];

    if (res -> type == RESOURCE_IO)
    {
      if (pci_resource_get_base(dev, i) != 0)
        return pci_resource_get_base(dev, i);
    }
  }

  return 0;
}
