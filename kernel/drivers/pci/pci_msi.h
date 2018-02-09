#ifndef __MSI_H__
#define __MSI_H__

#include <common.h>

struct msix
{
  u8  enabled;
  u8  function_mask;
  u32 cap_base;

  u32 max_entries;
  u32 allocated_entries;

  u32 *table_addr;
  u32 *pba_addr;

  b8 per_vector_masking:1;
  b8 big_addr:1;
};

struct msix_capability_header
{
  u8  capability_id;
  u8  next_pointer;
  u16 message_controll;

  u32 table_offset;
  u32 pba_offset;
} att_packed;

struct msix_table_entry
{
  u32 msg_addr;
  u32 msg_addr_upper;
  u32 msg_data;
  u32 vector_ctrl;
} att_packed;

#define MSIX_CTRL_MASK_BIT 0x1

void
msi_set_vector(struct msix *msix, u32 cpu, u32 intr);

#endif