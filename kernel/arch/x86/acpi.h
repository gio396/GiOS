#ifndef __APCI_H__
#define __APCI_H__

#include <common.h>

//TODO add another RSDT entries;
#define RSDT_MADT "APIC"


struct rsdp_descriptor
{
  i8 signature[8];
  u8 checksum;
  i8 oem_id[6];
  u8 revision;
  u32 rsdt_addr;
} att_packed;

struct acpi_sdt_header
{
  i8 signature[4];
  u32 length;
  u8 revision;
  u8 checksum;
  i8 oem_id[6];
  i8 oem_table_id[8];
  u32 oem_revision;
  u32 creator_id;
  u32 creatorRevision;
} att_packed;

struct rsdt
{
  struct acpi_sdt_header header;
  u32 pointer_to_other_sdt[];
};

struct madt_entry_header
{
  u8 entry_type;
  u8 entry_length;
} att_packed;

//single physical processor and it's local interrupt controller
struct madt_entry0
{
  struct madt_entry_header header;

  u8 apic_processor_id;
  u8 apic_id;
  u32 flags;
} att_packed;

//represents IOAPIC
struct madt_entry1
{
  struct madt_entry_header header;

  u8 ioapic_id;
  u8 reserved;
  u32 ioapic_addr;
  u32 global_system_interrupt_base;
} att_packed;

//Contains data for and Interrupt Source Override
struct madt_entry2
{
  struct madt_entry_header header;

  u8 bus_source;
  u8 irq_source;
  u32 global_system_interrupt;
  u16 flags;
} att_packed;

//nonmaskable interrupt soruce
struct madt_entry3
{
  struct madt_entry_header header;

  u16 flags;
  u32 global_system_interrupt;
} att_packed;

//local APIC MNI structrue.
struct madt_entry4
{
  struct madt_entry_header header;

  u8 apic_processor_id;
  u16 flags;
  u8 local_apic_inti;
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

  u32 local_controller_adress;
  u32 flags;

  union madt_entry first_entry;
} att_packed;

void
find_rsdp();

void*
find_rstd_descriptor(i8* signature);

#endif
