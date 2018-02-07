#ifndef __PCI_H__
#define __PCI_H__

#include <common.h>
#include <rbtree.h>
#include <list.h>

#include <arch/x86/register.h>

#include <drivers/device/device.h>
#include <drivers/device/device_bus.h>
#include "pci_msi.h"

//NOTE(gio): We use configuration space access mechanism #1 for know should probe int 0x1A to get supported access mechanism.
//NOTE(gio): We should probe ACPI tables and check if memory mapped configuration space access is supported for acess mechanism #1!.

#define PCI_VENDOR_OFFSET               0x00
#define PCI_DEVICE_ID_OFFSET            0x02
#define PCI_CMD_REG_OFFSET              0x04
#define PCI_STATUS_OFFSET               0x06
#define PCI_CLASS_OFFSET                0x0a
#define PCI_REVISION_ID_OFFSET          0x08
#define PCI_BASE_CLASS_OFFSET           0x0b
#define PCI_HEADER_TYPE_OFFSET          0x0d
#define PCI_SUBSYSTEM_ID_OFFSET         0x2e
#define PCI_BASE_ADRESS_OFFSET(x)       (0x10 + (x) * 0x04)
#define PCI_CAPABILITIES_POINTER_OFFSET 0x34
#define PCI_INT_LINE_OFFSET             0x3c

#define PCI_01H_SECONDARY_BUS_OFFSET 0x18

#define PCI_GET_BASE_CLASS(bc) ((bc >> 8) & 0x00ff)
#define PCI_GET_SUB_CLASS(bc)  ((bc) & 0x00ff)

#define BASE_ADDRESS_MEM_MASK (~0x0FU)
#define BASE_ADDRESS_IO_MASK  (~0x03U)

#define PCI_MSI_CAP                    0x05
#define PCI_MSIX_CAP                   0x11

#define PCI_COMMAND_MASTER 0x04
#define PCI_COMMAND_MEM    0x02
#define PCI_COMMAND_IO     0x01

#define UNIQUE(in) __UNIQ__##in##__##__COUNTER__
#define INIT_CORE_EXPORT(coreid, func) __attribute__((section(".cinit" #coreid))) void* UNIQUE(func) = (void*)func
#define INIT_CORE0_EXPORT(func) INIT_CORE_EXPORT(0, func)

#define PCI_VENDOR_ANY    0xffff
#define PCI_DEVICE_ANY    0xffff
#define PCI_SUBSYSTEM_ANY 0xffff
#define PCI_SUB_CLASS_ANY  0xff
#define PCI_CLASS_ANY     0xff
#define PCI_REVISION_ANY  0xff

struct pci_bus
{
  u8 bus;

  struct device_bus *pci_device_bus;
  struct pci_bus    *parrent;
  struct dlist_root children;

  struct dlist_root devices;
  struct dlist_node next;

  struct pci_dev *self;

  struct dlist_root pci_drivers;
};

#define RESOURCE_IO  1
#define RESOURCE_MEM 2

#define pci_resource_get_base(d, i) (d) -> resources[(i)].start
#define pci_resource_get_end(d, i)  (d) -> resources[(i)].end
#define pci_resource_get_legth(d, i)                              \
        pci_resource_get_base(d, i) == 0 ? 0:                     \
        pci_resource_get_end(d, i)  == 0 ? 0:                     \
        (pci_resource_get_end(d,i) - pci_resource_get_base(d,i))  \


struct pci_dev_resource
{
  u8 type;
  u32 start;
  u32 end;
};

struct pci_dev
{
  u8 dev;
  u8 func;

  u16 vendor_id;
  u16 device_id;
  u16 subsystem_id;

  u8 base_class;
  u8 sub_class;
  u8 revision;

  u8 header_type;
  u8 capabilities;
  b8 has_msix;
  b8 should_use_msix;
  u8 irq;

  u32 BAR[6];
  struct pci_dev_resource resources[6];
  struct msix msix;
  //...

  struct dlist_node self;

  struct pci_bus *bus;
  struct pci_driver *driver;
  struct device device;
};

struct pci_id
{
  u16 vendor;
  u16 device;
  u16 subsystem;
  u8 class;
  u8 sub_class;
  u8 revision;
};

struct pci_driver
{
  struct device_driver device_driver;

  i8 *name;
  struct pci_id *id_list;

  b8 (*match)(struct pci_dev *dev);
  struct pci_dev* (*init)(struct pci_dev *dev);
  b8 (*probe)(struct pci_dev *dev);
  b8 (*setup)(struct pci_dev *dev);
  void (*remove)(struct pci_dev *dev);
  void (*ievent)(const union biosregs *iregs, struct pci_dev *dev);

  struct dlist_node self;
};

//NOTE(gio): if  Device doesn't exist read will return all ones.
u16
pci_config_read_word(u8 bus, u8 slot, u8 func, u8 offset);

void
pci_init();

void
pci_enum();

struct dlist_node*
pci_lookup_device_start();

struct dlist_node*
pci_lookup_device_next(struct dlist_node *it, u16 vendor_id);

struct dlist_node*
pci_lookup_device(struct dlist_node *it, u16 vendor_id);

void
pci_dev_read_config_byte(struct pci_dev *dev, u8 offset, u8 *res);

void
pci_dev_read_config_word(struct pci_dev *dev, u8 offset, u16 *res);

void
pci_dev_read_config_dword(struct pci_dev *dev, u8 offset, u32 *res);

void
pci_dev_write_config_word(struct pci_dev *dev, u8 offset, u16 value);

i32
pci_dev_find_capability(struct pci_dev *dev, u8 cap);

u32
pci_dev_get_iobase(struct pci_dev *dev);

void
pci_msix_enable(struct pci_dev *dev);

void
pci_register_driver(struct pci_driver *driver);

#endif
