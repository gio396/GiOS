#ifndef __PCI_H__
#define __PCI_H__

#include <common.h>

#include <list.h>

//NOTE(gio): We use configuration space access mechanism #1 for know should probe int 0x1A to get supported access mechanism.
//NOTE(gio): We should probe ACPI tables and check if memory mapped configuration space access is supported for acess mechanism #1!.


struct pci_bus
{
  struct pci_bus    *parrent;
  struct dlist_root children;

  struct dlist_root devices;
  struct dlist_node next;

  struct pci_dev *self;
  u8 bus;
};

struct pci_dev
{
  struct pci_bus *bus;
  u8 base_class;
  u8 sub_class;
  u8 header_type;
  //...

 struct dlist_node self;
};

struct pci_bus *global_bus;

//NOTE(gio): if  Device doesn't exist read will return all ones.
u16
pci_config_read_word(u8 bus, u8 slot, u8 func, u8 offset);

void
pci_init_enum();

#endif
