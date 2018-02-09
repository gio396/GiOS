#include "pci_msi.h"

#include <macros.h>
#include <list.h>

#include <drivers/pci/pci.h>
#include <arch/x86/framebuffer.h>


u32
msi_mm_read(u32 reg)
{
  u32* addr = (u32*)reg;
  return addr[0];
}

void
msi_mm_write(u32 reg, u32 val)
{
  u32 *addr = (u32*)reg;
  addr[0] = val;
}

size_t
msi_get_table_reg(struct msix *msix, u32 vec, u32 offset)
{
  size_t res = (size_t)(msix -> table_addr) + vec * sizeof(struct msix_table_entry) + offset;

  return res;
}

void
msix_mask_vector(struct msix *msix, u32 vec)
{
  size_t reg = msi_get_table_reg(msix, vec, OFFSET_OF(struct msix_table_entry, vector_ctrl));
  u32 ctrl = msi_mm_read(reg);
  msi_mm_write(reg, ctrl & ~MSIX_CTRL_MASK_BIT);
}

void
msix_unmask_vector(struct msix *msix, u32 vec)
{
  size_t reg = msi_get_table_reg(msix, vec, OFFSET_OF(struct msix_table_entry, vector_ctrl));
  u32 ctrl = msi_mm_read(reg);
  msi_mm_write(reg, ctrl | MSIX_CTRL_MASK_BIT);
}

u32
msix_addr_1_cpu(u32 cpu)
{
  u32 RH = 1;
  u32 DM = 0;

  return 0xfee00000 | (cpu << 12) | (RH << 3) | (DM << 2);
}

u32
msix_msg_data_vector(u32 intr)
{
  u32 DM = 0;
  u32 TM = 0;

  return (TM << 15) | (DM << 8) | intr;
}

#include <timer.h>

void
msi_redirect_vector(struct msix *msix, u32 vec, u32 cpu, u32 intr)
{
  msix_mask_vector(msix, vec);

  msi_mm_write(msi_get_table_reg(msix, vec, OFFSET_OF(struct msix_table_entry, msg_addr)), msix_addr_1_cpu(cpu));
  msi_mm_write(msi_get_table_reg(msix, vec, OFFSET_OF(struct msix_table_entry, msg_addr_upper)), 0);
  msi_mm_write(msi_get_table_reg(msix, vec, OFFSET_OF(struct msix_table_entry, msg_data)), msix_msg_data_vector(intr));

  msix_unmask_vector(msix, vec);
}

void
msi_set_vector(struct msix* msix, u32 cpu, u32 intr)
{
  u16 vec;

  for (vec = 0; vec < msix -> max_entries; vec++)
  {
    size_t reg = msi_get_table_reg(msix, vec, OFFSET_OF(struct msix_table_entry, msg_data));
    if ((msi_mm_read(reg) & 0xff) == 0){break;}
  }

  if (vec == msix -> max_entries)
    return;

  msi_redirect_vector(msix, vec, cpu, intr);

}
