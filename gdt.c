#include "gdt.h"

#include "common.h"
#include "framebuffer.h"
#include "io.h"


#define SEG_PRES(x) ((x) << 0x0F) // 1 present 0 not present
#define SEG_PRIV(x) ((x) << 0x0E) // privilage ring (0 - 3)
#define SEG_DSCT(x) ((x) << 0x0C) // 1 system 0 code\data
#define SEG_GRAN(x) ((x) << 0x07) // 1 4kb mode 0 1byte mode
#define SEG_OPSZ(x) ((x) << 0x06) // 1 32bit 0 16bit
#define SEG_LONG(x) ((x) << 0x05) // long mode
#define SEG_SAVL(x) ((x) << 0x04) // available for system?

#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed

#define GDT_CODE_PL0    SEG_PRES(1) | SEG_PRIV(0) | SEG_DSCT(1) | \
                        SEG_GRAN(1) | SEG_OPSZ(1) | SEG_LONG(0) | \
                        SEG_SAVL(0) | SEG_CODE_EXRD

#define GDT_DATA_PL0    SEG_PRES(1) | SEG_PRIV(0) | SEG_DSCT(1) | \
                        SEG_GRAN(1) | SEG_OPSZ(1) | SEG_LONG(0) | \
                        SEG_SAVL(0) | SEG_DATA_RDWR

#define GDT_CODE_PL3    SEG_PRES(1) | SEG_PRIV(3) | SEG_DSCT(1) | \
                        SEG_GRAN(1) | SEG_OPSZ(1) | SEG_LONG(0) | \
                        SEG_SAVL(0) | SEG_CODE_EXRD


#define GDT_DATA_PL3    SEG_PRES(1) | SEG_PRIV(3) | SEG_DSCT(1) | \
                        SEG_GRAN(1) | SEG_OPSZ(1) | SEG_LONG(0) | \
                        SEG_SAVL(0) | SEG_DATA_RDWR


//sets gdt_gate at num index.
//      16 15    13  12    8  7   6  5  4        0
//flags [P-|DPL--|DT-|0----|G-|DB-|0-|A-|TYPE----]
internal void
gdt_set_gate(int32 num, uint32 base, uint32 limit, uint16 flags)
{
  printk(&state, "Setting gdt %d, base 0x%8X, limit 0x%8X, flags %16b\n",
         num, base, limit, flags);

  gdt[num].base_low     = (base & 0x0000FFFF);
  gdt[num].base_middle  = (base & 0x00FF0000);
  gdt[num].base_high    = (base & 0xFF000000);


  gdt[num].limit_low    = (limit & 0x0000FFFF);

  //high 4 bits for segment length 19:16
  gdt[num].granularity  = (limit & 0x000F);
  gdt[num].granularity |= (flags & 0x00F0); //[G-|DB-|0-|A-]

  gdt[num].access = ((flags & 0xF000) >> 8) |
                    (flags & 0x000F);       //[P-|DPL--|DT-|TYPE----|
}

void 
gdt_install()
{
  printk(&state, "Creating GDT table\n");

  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base = (int32)&gdt;

  printk(&state, "GDT limit 0x%4X. base 0x%8X\n",
         gp.limit, gp.base);

  // null descriptor
  gdt_set_gate(0,0,0,0);

  //code segment 4gb, 4kb aligned
  gdt_set_gate(1, 0, 0xFFFFFFFF, GDT_CODE_PL0);

  //data segment 4gb, 4kb aligned
  gdt_set_gate(2, 0, 0xFFFFFFFF, GDT_DATA_PL0);

  //userspace code segment 4gb, 4kb alligned
  gdt_set_gate(3, 0, 0xFFFFFFFF, GDT_CODE_PL3);

  //userspace data segment 4gb, 4kb alligned
  gdt_set_gate(4, 0, 0xFFFFFFFF, GDT_DATA_PL3);

  gdt_flush((int32)&gp);
  printk(&state, "Flushed GP at 0x%8X\n", (int32)&gp);
}