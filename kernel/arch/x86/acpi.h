#ifndef __APCI_H__
#define __APCI_H__

#include <common.h>

//TODO add another RSDT entries;
#define RSDT_MADT "APIC"


struct rsdp_descriptor
{
  int8 signature[8];
  uint8 checksum;
  int8 oem_id[6];
  uint8 revision;
  uint32 rsdt_addr;
} att_packed;

struct acpi_sdt_header
{
  int8 signature[4];
  uint32 length;
  uint8 revision;
  uint8 checksum;
  int8 oem_id[6];
  int8 oem_table_id[8];
  uint32 oem_revision;
  uint32 creator_id;
  uint32 creatorRevision;
} att_packed;

struct rsdt
{
  struct acpi_sdt_header header;
  uint32 pointer_to_other_sdt[];
};

struct madt_entry_header
{
  uint8 entry_type;
  uint8 entry_length;
} att_packed;

//single physical processor and it's local interrupt controller
struct madt_entry0
{
  struct madt_entry_header header;

  uint8 apic_processor_id;
  uint8 apic_id;
  uint32 flags;
} att_packed;

//represents IOAPIC
struct madt_entry1
{
  struct madt_entry_header header;

  uint8 ioapic_id;
  uint8 reserved;
  uint32 ioapic_addr;
  uint32 global_system_interrupt_base;
} att_packed;

//Contains data for and Interrupt Source Override
struct madt_entry2
{
  struct madt_entry_header header;

  uint8 bus_source;
  uint8 irq_source;
  uint32 global_system_interrupt;
  uint16 flags;
} att_packed;

//nonmaskable interrupt soruce
struct madt_entry3
{
  struct madt_entry_header header;

  uint16 flags;
  uint32 global_system_interrupt;
} att_packed;

//local APIC MNI structrue.
struct madt_entry4
{
  struct madt_entry_header header;

  uint8 apic_processor_id;
  uint16 flags;
  uint8 local_apic_inti;
} att_packed;

union madt_entry
{
  struct madt_entry0 ent0;
  struct madt_entry1 ent1;
  struct madt_entry2 ent2;
  struct madt_entry3 ent3;
  struct madt_entry4 ent4;
};

struct madt
{
  struct acpi_sdt_header header;

  uint32 local_controller_adress;
  uint32 flags;

  union madt_entry first_entry;
} att_packed;

void
find_rsdp();

void*
find_rstd_descriptor(int8* signature);

#endif
