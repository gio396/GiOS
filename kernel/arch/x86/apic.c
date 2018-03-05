#include "apic.h"

#include <arch/x86/framebuffer.h>
#include <arch/x86/msr.h>
#include <arch/x86/acpi.h>
#include <arch/x86/irq.h>
#include <arch/x86/idt.h>
#include <arch/x86/pit.h>
#include <arch/x86/io.h>

#include <macros.h>

//63      MAXPHYSADDR       11  10   8    7    0
//[R--------*-------|BASE-32-|GE-|R--|BSP-|R-7-]

#define IA32_APIC_BASE_MSR_BSP    0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define IOAPIC_REG_ID             0x00
#define IOAPIC_REG_VERSION        0x01
#define IOAPIC_REG_ARB_ID         0x02
#define IOAPIC_REG_RED_TLB_BASE   0x10
#define IOAPIC_REG_RED_TLB_HEAD   0x3F

//IOAPIC redirection table entry
//64        55  16      15     14    13     12      11      10     7      0
//[DESTFIELD|RES|INTMASK|TRMODE|IRRRO|INTPOL|DELSTAT|DESTMOD|DELMOD|INT_VEC]

#define IOAPIC_SEG_INT_VEC(x)     (x)
#define IOAPIC_SEG_DEL_MOD(x)     ((x) << 0x08)
#define IOAPIC_SEG_DES_MOD(x)     ((x) << 0x0b)
#define IOAPIC_SEG_DEL_STA(x)     ((x) << 0x0c)
#define IOAPIC_SEG_INT_POL(x)     ((x) << 0x0d)
#define IOAPIC_SEG_IRR_RO(x)      ((x) << 0x0e)
#define IOAPIC_SEG_TRI_MOD(x)     ((x) << 0x0f)
#define IOAPIC_SEG_INT_MAS(x)     ((x) << 0x10)

#define IOAPIC_DEST_FIELD(x)      ((x) << 0x17)

#define APIC_TIMER_ONESHOT  (0x0 << 17)
#define APIC_TIMER_PERIODIC (0x1 << 17)
#define APIC_TIMER_DEADLINE (0x2 << 17)

#define APIC_DIVIDE1   0xB
#define APIC_DIVIDE2   0x0
#define APIC_DIVIDE4   0x1
#define APIC_DIVIDE8   0x2
#define APIC_DIVIDE16  0x3
#define APIC_DIVIDE32  0x8
#define APIC_DIVIDE64  0x9
#define APIC_DIVIDE128 0xA

#define INITIAL_TIMER_COUNT 200000000
#define APIC_TIMER_DEST_VEC 48
#define APIC_TARGET_FREQUENCY 1000000

global i32 apic_freq = 0;
global i32 apic_tick_count = 1;

extern void (*irq_handler_pointer)(union biosregs *iregs);

extern void new_irq_handler();

global struct 
{
  u8 ioapic_id;
  u32 ioapic_addr;
} ioapic;

void
apic_write_reg(u32 reg, u32 val)
{
  u32 offset = reg;
  u32 volatile *reg_addr = (u32*)(APIC_LOCATION + offset);

  reg_addr[0] = val;
}

u32
apic_read_reg(u32 reg)
{
  u32 offset = reg;
  u32 *reg_addr = (u32*)(APIC_LOCATION + offset);

  return(*reg_addr);
}

void 
ioapic_write_reg(u8 offset, u32 val)
{
  u8 *ioapic_base_cmd = (u8*)(ioapic.ioapic_addr);
  u32 *ioapic_base_data = (u32*)(ioapic.ioapic_addr + 0x10);

  ioapic_base_cmd[0] = offset;
  ioapic_base_data[0] = val;
}

void 
ioapic_read_reg(u8 offset, u32* val)
{
  u8 *ioapic_base_cmd = (u8*)(ioapic.ioapic_addr);
  u32 *ioapic_base_data = (u32*)(ioapic.ioapic_addr + 0x10);

  ioapic_base_cmd[0] = offset;
  *val = ioapic_base_data[0];
}

u32
lzcnt(u32 val)
{
  u32 res;
  __lzcnt(res, val);

  return res;
}

u32
apic_read_isr()
{
  for (i32 i = 8; i >= 1; i--)
  {
    u32 reg = apic_read_reg(APIC_IN_SERVICE0_REGISTER + 0x10 * i);
    if (reg) return 32 * i + lzcnt(reg);
  }

  return 256;
}

void
apic_irq_handler(union biosregs *iregs)
{
  u8 vector = apic_read_isr();

  iregs -> int_no = vector;
  idt_call_irq(vector, iregs);

  irq_eoi(1, vector);
}

void  
apic_init_timer(void)
{
  const i32 pit_end_count = 10000;
  const i32 pit_initial_count = 60000;
  const u32 apic_initial_count = 100000000;
  const u32 masked_apic_16 = IOAPIC_SEG_INT_VEC(APIC_TIMER_DEST_VEC) | IOAPIC_SEG_INT_MAS(1) | APIC_TIMER_PERIODIC;
  const u32 unmasked_oneshot = IOAPIC_SEG_INT_VEC(APIC_TIMER_DEST_VEC) | IOAPIC_SEG_INT_MAS(0) | APIC_TIMER_ONESHOT;

  u32 apic_test_count;

  apic_write_reg(APIC_DIVIDE_CONFIGURATION_REGISTER, APIC_DIVIDE16);
  apic_write_reg(APIC_INITIAL_COUNT_REGISTER, apic_initial_count);

  apic_write_reg(APIC_LVT_TIMER_REGISTER, masked_apic_16);

  pit_interrupt_in(pit_initial_count);

  apic_write_reg(APIC_LVT_TIMER_REGISTER, masked_apic_16 ^ IOAPIC_SEG_INT_MAS(1));

  volatile i32 cnt = 0;
  do
  {

    //nop to let pit count
    //when running in virtual machine;
    for (int i = 0; i < 100; i++)
    {
      __asm__ __volatile__ ("NOP");
    }

    cnt = pit_get_current_count();
  } while(cnt >= pit_end_count);

  //mask apic.
  apic_write_reg(APIC_LVT_TIMER_REGISTER, masked_apic_16);

  //get apic titmer count.
  apic_test_count = apic_read_reg(APIC_CURRENT_COUNT_REGISTER);

  printk(&state, "count %d\n", (apic_initial_count - apic_test_count));

  apic_freq = ( (PIT_DEF_FREQUENCY / (pit_initial_count - cnt) ) * (apic_initial_count - apic_test_count));
  printk(&state, "APIC freq ~ %d\n", apic_freq);

  // ticks per 1us
  apic_tick_count = (apic_freq + APIC_TARGET_FREQUENCY - 1) / (APIC_TARGET_FREQUENCY);

  printk(&state, "Target Frequency %d\n", apic_tick_count);

  apic_write_reg(APIC_INITIAL_COUNT_REGISTER, 0);

  apic_write_reg(APIC_LVT_TIMER_REGISTER, unmasked_oneshot);
  printk(&state, "Apic Timer initialization done!\n");
}

void
apic_timer_interrupt_in(u32 us)
{
  disable_interrupts();

  u32 count = (us * apic_tick_count);

  apic_write_reg(APIC_INITIAL_COUNT_REGISTER, count);

  enable_interrupts();
}

u32
apic_timer_get_count(void)
{
  return apic_read_reg(APIC_CURRENT_COUNT_REGISTER); 
}

u32
apic_timer_get_tick_count(void)
{
  return (apic_timer_get_count() / apic_tick_count);
}


void
apic_enable(u32 apic_base_address)
{
  u32 apic_base_register_lo;
  u32 apic_base_register_hi;
  u32 apic_id; 

  apic_base_register_lo = (apic_base_address & 0xFFFFF000) | IA32_APIC_BASE_MSR_ENABLE;
  apic_base_register_hi = 0;

  (void)(apic_base_register_lo);
  (void)(apic_base_register_hi);

  //TODO(gio): This call causes GPE when running with kvm enabled.
  // cpu_set_msr(IA32_APIC_BASE_MSR, apic_base_register_hi, apic_base_register_lo);

  apic_write_reg(APIC_DESTINATION_FORMAT_REGISTER, 0xFFFFFFFF);
  apic_write_reg(APIC_LOGICAL_DESTINATION_REGISTER, 0x10000000);

  apic_write_reg(APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER, 
                 apic_read_reg(APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER) | 0x100
                );

  apic_id = apic_read_reg(APIC_ID_REGISTER);

  printk(&state, "APIC id: %08x\n", apic_id);

  parse_madt_table();

  u32 version;
  ioapic_read_reg(IOAPIC_REG_VERSION, &version);
  printk(&state, "IOAPIC Version = 0x%02X\n", version & 0xff);
  printk(&state, "IOAPIC Max RT entries = 0x%02x\n", (version >> 16) & 0xff);


  //keyboard rte
  u32 keyboard_lo = IOAPIC_SEG_INT_VEC(0x21) | IOAPIC_SEG_DEL_MOD(0x00) | IOAPIC_SEG_DES_MOD(0x00) | 
                                               IOAPIC_SEG_DEL_STA(0x00) | IOAPIC_SEG_INT_POL(0x00) | IOAPIC_SEG_IRR_RO (0x00) |
                                               IOAPIC_SEG_TRI_MOD(0x00) | IOAPIC_SEG_INT_MAS(0x00);
  ioapic_register_rte(1, 0x00, keyboard_lo);

  //apic handler;
  irq_handler_pointer = apic_irq_handler;
}

void
parse_madt_table(void)
{
  void* descriptor = find_rstd_descriptor(RSDT_MADT);

  printk(&state, "descriptor = %08X\n", descriptor);

  struct madt *madt = (struct madt*)(descriptor);
  void *madt_end = (i8*)madt + madt->header.length;
  
  for (union madt_entry *cur = &madt->first_entry; (void*)cur < madt_end;)
  {
    switch(cur->ent0.header.entry_type)
    {
      case 0:
      {
        printk(&state, "Printing madt of type 0\n");
        printk(&state, "  ACPI processor Id   0x%02X\n", cur->ent0.apic_processor_id);
        printk(&state, "  ACPI id             0x%02X\n", cur->ent0.apic_id);
        printk(&state, "  flags               0x%08X\n", cur->ent0.flags);
        break;
      }

      case 1:
      {
        printk(&state, "Printing madt of type 1\n");
        printk(&state, "  IOAPIC id:          0x%02X\n", cur->ent1.ioapic_id);
        printk(&state, "  IOAPIC address      0x%04X\n", cur->ent1.ioapic_addr);
        printk(&state, "  Global SYS INT base 0x%08X\n", cur->ent1.global_system_interrupt_base);

        ioapic.ioapic_addr = cur->ent1.ioapic_addr;
        ioapic.ioapic_id = cur->ent1.ioapic_id;
        break;
      }

      case 2:
      {
        printk(&state, "Printing madt of type 2\n");
        printk(&state, "  Bus source          0x%02X\n", cur->ent2.bus_source);
        printk(&state, "  irq source          0x%02X\n", cur->ent2.irq_source);
        printk(&state, "  Global SYS INT      0x%08X\n", cur->ent2.global_system_interrupt);
        printk(&state, "  flags               0x%04X\n", cur->ent2.flags);
        break;
      }

      case 3:
      {
        printk(&state, "Printing madt of type 3\n");
        printk(&state, "  Global SYS INT      0x%08X\n", cur->ent3.global_system_interrupt);
        printk(&state, "  flags               0x%04X\n", cur->ent3.flags);
        break;
      }

      case 4:
      {
        printk(&state, "Printing madt of type 4\n");
        printk(&state, "  ACPI processor Id   0x%02X\n", cur->ent4.apic_processor_id);
        printk(&state, "  local apic inti#    0x%04X\n", cur->ent4.local_apic_inti);
        printk(&state, "  flags               0x%02X\n", cur->ent4.flags);
        break;
      }
    }


    cur = (union madt_entry*)((i8*)cur + cur->ent0.header.entry_length);
  }
}

void
ioapic_register_rte(u32 irq, u32 cpu, u32 lo)
{
  //NOTE(GIO:) Only physical destination ids and single cpu!
  ioapic_write_reg(IOAPIC_REG_RED_TLB_BASE + irq * 2, lo);
  ioapic_write_reg(IOAPIC_REG_RED_TLB_BASE + irq * 2 + 1, IOAPIC_DEST_FIELD(cpu));
}
