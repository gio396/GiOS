#include "pci.h"

#include <arch/x86/io.h>
#include <arch/x86/framebuffer.h>
#include <arch/x86/page.h>
#include <arch/x86/idt.h>

#include <memory.h>

#include <timer.h>

#include <macros.h>
#include <string.h>

//Configuration Space Access Mechanism #1 adress.
// 31          30 - 24   23 - 16     15 - 11         10 - 8            7 - 2            1-0
// Enable Bit  Reserved  Bus Number  Device Number   Function Number   Register Number  00

#define MSG_CTRL_MASK_BIT (1 << 14)

#define PCI_MCTRL_BUS_MASTER (1 << 2)

struct pci_bus *global_bus;

struct pci_bus*
pci_append_bus(struct pci_bus *node, struct pci_bus *parrent)
{
  node -> parrent = parrent;
  dlist_insert_tail(&parrent -> children, &node -> next);

  return node;
}

void
pci_register_device(struct pci_dev *dev)
{
  bus_register_device(global_bus -> pci_device_bus, &dev -> device);
  dlist_insert_tail(&dev -> bus -> devices, &dev -> self);
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
  tmp = (u16)((inl(0xCFC) >> ((offset & 3) * 8)) & 0xff);

  return tmp;
}

u16
pci_config_read_word(u8 bus, u8 slot, u8 func, u8 offset)
{
  assert1(ALIGNED(offset, 0x1));

  u32 adress;
  u32 lbus  = bus;
  u32 lslot = slot;
  u32 lfunc = func;
  u32 tmp   = 0;

  adress = ((u32)0x80000000 | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc));

  outl(0xCF8, adress);
  tmp = (u16)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);

  return tmp;
}

u32
pci_config_read_dword(u8 bus, u8 slot, u8 func, u8 offset)
{
  assert1(ALIGNED(offset, 0x1));

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
pci_config_write_word(u8 bus, u8 slot, u8 func, u8 offset, u16 value)
{
  u32 adress;
  u32 lbus  = bus;
  u32 lslot = slot;
  u32 lfunc = func;

  adress = ((u32)0x80000000 | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfe));

  outl(0xCF8, adress);
  outs(0xCFC + (offset & 2), value);
}

void
check_device(u8 bus, u8 device, struct pci_bus *lbus)
{
  u8 function = 0;
  u16 header_type;
  u16 vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_OFFSET);

  if (vendor_id == 0xffff) return; //non-existant device

  struct pci_dev ldevice = {};

  check_function(bus, device, function, lbus, &ldevice);

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
        check_function(bus, device, function, lbus, &ldevice);
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

  if (pos == 0)
  {
    dev -> has_msix = 0;
    return 0;
  }

  dev -> has_msix = 1;
  dev -> msix.cap_base =  pos;

  return 1;
}

void
pci_dev_setup_msix(struct pci_dev *dev)
{
  u8 pos = dev -> msix.cap_base;
  u16 mctrl;
  pci_dev_read_config_word(dev, pos + OFFSET_OF(struct msix_capability_header, message_controll), &mctrl);

  u16 max_entries = (mctrl && 0x7ff) + 1;

  u32 toff;
  u32 pbaoff;
  pci_dev_read_config_dword(dev, pos + OFFSET_OF(struct msix_capability_header, table_offset), &toff);
  pci_dev_read_config_dword(dev, pos + OFFSET_OF(struct msix_capability_header, pba_offset), &pbaoff);

  u8 toff_bar = toff & 0x07;
  u8 pbaoff_bar = pbaoff & 0x07;

  dev -> msix.allocated_entries = 0;
  dev -> msix.max_entries = max_entries;
  dev -> msix.per_vector_masking = (mctrl >> 8) & 0x1;
  dev -> msix.big_addr  = (mctrl >> 7) & 0x1;
  dev -> msix.table_addr = (u32*)(pci_resource_get_base(dev, toff_bar) + (toff & (~0x07)));
  dev -> msix.pba_addr = (u32*)(pci_resource_get_base(dev, pbaoff_bar) + (pbaoff & (~0x07)));
  dev -> msix.enabled = 1;//(mctrl >> 15) & 0x1;
  dev -> msix.function_mask = (mctrl >> 14) & 1;

  if (dev -> msix.enabled)
  {
    mmap(dev -> msix.table_addr, 1, 0);
    mmap(dev -> msix.pba_addr,   1, 0);
  }

  mctrl = (mctrl & ~(1 << 14));
  pci_dev_write_config_word(dev, pos + OFFSET_OF(struct msix_capability_header, message_controll), mctrl);
}

internal b8
match_dri_dev(struct pci_driver *driver, struct pci_dev *dev)
{
  struct pci_id *it = driver -> id_list;

  while (it -> vendor != 0 || it -> device != 0 || it -> subsystem != 0)
  {
    if (it -> vendor != PCI_VENDOR_ANY && it -> vendor != dev -> vendor_id)
    {
      goto next;
    }

    if (it -> device != PCI_DEVICE_ANY && it -> device != dev -> device_id)
    {
      goto next;
    }

    if (it -> subsystem != PCI_SUBSYSTEM_ANY && it -> subsystem != dev -> subsystem_id)
    {
      goto next;
    }

    if (it -> class != PCI_CLASS_ANY && it -> class != dev -> base_class)
    {
      goto next;
    }

    if (it -> sub_class != PCI_SUB_CLASS_ANY && it -> sub_class  != dev -> sub_class)
    {
      goto next;
    }

    if (it -> revision != PCI_REVISION_ANY  && it -> revision != dev -> revision)
    {
      goto next;
    }

    return 1;

next:
  it = it + 1;
  }

  return 0;
}

internal b8
pci_find_driver(struct pci_driver **found, struct pci_dev *dev)
{
  struct pci_driver *last = *found;
  struct dlist_node *head = NULL;
  struct dlist_node *it = NULL;

  if (last == NULL)
  {
    head = global_bus -> pci_drivers.dlist_node;
  }
  else
  {
    head = last -> self.next;
  }

  FOR_EACH_LIST(it,head)
  {
    struct pci_driver *dri = CONTAINER_OF(it, struct pci_driver, self);
    if (match_dri_dev(dri, dev))
    {
      *found = dri;
      return 1;
    }
  }

  return 0;
}

void
pci_irq_handler(const union biosregs *iregs, struct pci_dev *dev)
{
  struct pci_driver *driver = dev -> driver;
  if (driver && driver -> ievent)
  {
    driver -> ievent(iregs, dev);
  }
}

internal b8
pci_setup_msix(struct pci_dev *dev)
{
  for (u32 i = 0; i < dev -> msix.max_entries; i++)
  {
    u32 irq = get_next_irq();
    subscribe_irq(irq, pci_irq_handler, dev);
    msi_set_vector(&dev -> msix, 0, irq);
  }

  return 1;
}

internal b8
driver_try_setup(struct pci_driver *driver, struct pci_dev **pdev)
{
  struct pci_dev *dev = *pdev;
  if (!driver -> match(dev))
  {
    return 0;
  }

  struct pci_dev *new_dev = driver -> init(dev);
  new_dev -> driver = driver;
  
  if (new_dev == NULL)
  {
    return 0;
  }

  if (!driver -> probe(new_dev))
  {
    driver -> remove(new_dev);
    return 0;
  }

  if (dev -> has_msix && dev -> msix.enabled)
  {
    pci_setup_msix(new_dev);
  }

  if (!driver -> setup(new_dev))
  {
    driver -> remove(new_dev);
    return 0;
  }

  *pdev = new_dev;

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
  u8 pcapabilites;
  u8 cmd; 
  u8 revision;

  u16 lclass = pci_config_read_word(bus, device, function, PCI_CLASS_OFFSET);
  base_class = PCI_GET_BASE_CLASS(lclass);
  sub_class = PCI_GET_SUB_CLASS(lclass);
  vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_OFFSET);
  device_id = pci_config_read_word(bus, device, function, PCI_DEVICE_ID_OFFSET);
  subsystem_id = pci_config_read_word(bus, device, function, PCI_SUBSYSTEM_ID_OFFSET);
  pcapabilites = pci_config_read_word(bus, device, function, PCI_CAPABILITIES_POINTER_OFFSET) & 0x00ff;
  revision = pci_config_read_word(bus, device, function, PCI_REVISION_ID_OFFSET) & 0x00ff;
  cmd = pci_config_read_dword(bus, device, function, PCI_CMD_REG_OFFSET);

  cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEM | PCI_COMMAND_IO;
  cmd &= ~(PCI_INTERRUPT_ENABLE);

  pci_config_write_dword(bus, device, function, PCI_CMD_REG_OFFSET, cmd);

  cmd = pci_config_read_dword(bus, device, function, PCI_CMD_REG_OFFSET);

  LOGV("PCI %p", cmd);
  LOG("   Vendor id 0x%04x\n", vendor_id);
  LOG("   Device id 0x%04x\n", device_id);
  LOG("   Class base 0x%0x\n   class sub 0x%0x\n", base_class, sub_class);
  LOG("   Revision 0x%0x\n", revision);

  ldev -> bus          = lbus;
  ldev -> dev          = device;
  ldev -> func         = function;
  ldev -> vendor_id    = vendor_id;
  ldev -> device_id    = device_id;
  ldev -> base_class   = base_class;
  ldev -> sub_class    = sub_class;
  ldev -> subsystem_id = subsystem_id;
  ldev -> revision     = revision;
  ldev -> capabilities = pcapabilites;

  // fill resources!
  for (i32 i = 0; i < 6; i++)
  {
    ldev -> BAR[i] = pci_config_read_dword(bus, device, function, PCI_BASE_ADRESS_OFFSET(i));
    // LOG("BAR[%d] = 0x%08x.\n", i, ldev -> BAR[i]);
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

      ldev -> resources[i].type = RESOURCE_MEM;
      ldev -> resources[i].start = base_addr;
      ldev -> resources[i].end = pci_end;
    }
  }

  if (pci_dev_check_msix_capability(ldev) == 1)
  {
    pci_dev_setup_msix(ldev);
  }

  struct pci_driver *driver = NULL;
  struct pci_dev *new_device = ldev;

  while(pci_find_driver(&driver, ldev))
  {
    if (driver_try_setup(driver, &new_device) != 0)
      break;
  }

  if (new_device -> driver == NULL)
  {
    //No driver create pci_device store and register into the bus without driver;
    new_device = (struct pci_dev*)kzmalloc(sizeof(struct pci_dev));
    *new_device = *ldev;
  }

  //TODO(gio): Handle bus-bus bridges!
  // if ((base_class == 0x60) && (sub_class == 0x04))
  // {
  //   sbus = (pci_config_read_word(bus, device, function, PCI_01H_SECONDARY_BUS_OFFSET) >> 8) && 0x00ff;

  //   struct pci_bus *nbus = pci_append_bus(lbus);
  //   nbus -> self = ldev;

  //   walk_bus(sbus, nbus);
  // }

  pci_register_device(new_device);
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
    LOG("->(0x%p venodor:0x%4x device:0x%4x) 0x%02x%02x subsystem 0x%04x:", dev, dev -> vendor_id, dev -> device_id, dev -> base_class, dev -> sub_class, dev -> subsystem_id);
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
pci_enum()
{
  u8 function = 0;
  u8 bus;

  u16 header_type = pci_config_read_word(0, 0, 0, PCI_HEADER_TYPE_OFFSET);

  if ((header_type & 0x80) == 0)
  {
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
  it = it -> next;

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
  u8 device = dev -> dev;
  u8 func = dev -> func;
  u16 tmp;

  tmp = pci_config_read_byte(bus, device, func, offset);

  *res = (u8)(tmp & 0xff);
}

void
pci_dev_read_config_word(struct pci_dev *dev, u8 offset, u16 *res)
{
  u8 bus  = dev -> bus -> bus;
  u8 device = dev -> dev;
  u8 func = dev -> func;
  u16 tmp;

  tmp = pci_config_read_word(bus, device, func, offset);

  *res = tmp;
}

void
pci_dev_write_config_word(struct pci_dev *dev, u8 offset, u16 val)
{
  u8 bus = dev -> bus -> bus;
  u8 device = dev -> dev;
  u8 func = dev -> func;

  pci_config_write_word(bus, device, func, offset, val);
}

void
pci_dev_read_config_dword(struct pci_dev *dev, u8 offset, u32 *res)
{
  u8 bus  = dev -> bus -> bus;
  u8 device = dev -> dev;
  u8 func = dev -> func;
  u32 tmp;

  tmp = pci_config_read_dword(bus, device, func, offset);

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

  pos = pci_config_read_byte(bus, device, function, pos);

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
  u8 device = dev -> dev;
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

void
pci_msix_enable(struct pci_dev *dev)
{
  u8 pos = dev -> msix.cap_base;

  u16 mctrl;
  pci_dev_read_config_word(dev, pos + OFFSET_OF(struct msix_capability_header, message_controll), &mctrl);
  mctrl = mctrl & (~MSG_CTRL_MASK_BIT);
  pci_dev_write_config_word(dev, pos + OFFSET_OF(struct msix_capability_header, message_controll), mctrl);
}

void
pci_register_driver(struct pci_driver *driver)
{
  bus_register_driver(global_bus -> pci_device_bus, &driver -> device_driver);

  struct dlist_root *root = &global_bus -> pci_drivers;
  dlist_insert_tail(root, &driver -> self);
}

void
pci_init()
{
  global_bus = (struct pci_bus* )kzmalloc(sizeof(struct pci_bus));

  global_bus -> pci_device_bus = register_device_bus("pci_bus");
}

u16
pci_get_status(struct pci_dev *dev)
{
  u16 status;
  pci_dev_read_config_word(dev, PCI_STATUS_OFFSET, &status);

  return status;
}