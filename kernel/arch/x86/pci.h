#ifndef __PCI_H__
#define __PCI_H__

#include <common.h>

#include <list.h>

//NOTE(gio): We use configuration space access mechanism #1 for know should probe int 0x1A to get supported access mechanism.
//NOTE(gio): We should probe ACPI tables and check if memory mapped configuration space access is supported for acess mechanism #1!.


#define PCI_VENDOR_OFFSET               0x00
#define PCI_DEVICE_ID_OFFSET            0x02
#define PCI_STATUS_OFFSET               0x06
#define PCI_CLASS_OFFSET                0x0a
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

struct pci_bus
{
  struct pci_bus    *parrent;
  struct dlist_root children;

  struct dlist_root devices;
  struct dlist_node next;

  struct pci_dev *self;
  u8 bus;
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
  struct pci_bus *bus;
  u8 device;
  u8 func;
  u16 vendor_id;
  u16 device_id;
  u16 subsystem_id;
  u8 base_class;
  u8 sub_class;
  u8 header_type;
  u8 capabilities;
  u8 msix;
  u8 irq;
  u32 BAR[6];
  struct pci_dev_resource resources[6];
  //...
  struct dlist_node self;
};

struct pci_bus *global_bus;

//NOTE(gio): if  Device doesn't exist read will return all ones.
u16
pci_config_read_word(u8 bus, u8 slot, u8 func, u8 offset);

void
pci_init_enum();

struct dlist_node*
pci_lookup_device(struct dlist_node *it, u16 vendor_id, u16 device_id, u16 subsystem_id);

void
pci_dev_read_config_byte(struct pci_dev *dev, u8 offset, u8 *res);

void
pci_dev_read_config_word(struct pci_dev *dev, u8 offset, u16 *res);

i32
pci_dev_find_capability(struct pci_dev *dev, u8 cap);

u32
pci_dev_get_iobase(struct pci_dev *dev);

#endif
