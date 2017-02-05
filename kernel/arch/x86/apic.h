#ifndef __APIC_H__
#define __APIC_H__

#include <common.h> 

#define APIC_LOCATION                                    0xFEE00000

#define APIC_ID_REGISTER                                 0x020 //R W
#define APIC_VERSION_REGISTER                            0x030 //R
#define APIC_TASK_PRIORITY_REGISTER                      0x080 //R W
#define APIC_ARBITRATION_PRIORITY_REGISTER               0x090 //R
#define APIC_PROCESSOR_PRIORITY_REGISTER                 0x0A0 //R
#define APIC_EOI_REGISTER                                0x0B0 //  W
#define APIC_REMOTE_READ_REGISTER                        0x0C0 //R  
#define APIC_LOGICAL_DESTINATION_REGISTER                0x0D0 //R W
#define APIC_DESTINATION_FORMAT_REGISTER                 0x0E0 //R W
#define APIC_SPURIOUS_INTERRUPT_VECTOR_REGISTER          0x0F0 //R  
#define APIC_IN_SERVICE0_REGISTER                        0x100 //R  
#define APIC_IN_SERVICE1_REGISTER                        0x110 //R  
#define APIC_IN_SERVICE2_REGISTER                        0x120 //R  
#define APIC_IN_SERVICE3_REGISTER                        0x130 //R  
#define APIC_IN_SERVICE4_REGISTER                        0x140 //R  
#define APIC_IN_SERVICE5_REGISTER                        0x150 //R  
#define APIC_IN_SERVICE6_REGISTER                        0x160 //R  
#define APIC_IN_SERVICE7_REGISTER                        0x170 //R  
#define APIC_TRIGGER_MODE0_REGISTER                      0x180 //R  
#define APIC_TRIGGER_MODE1_REGISTER                      0x190 //R  
#define APIC_TRIGGER_MODE2_REGISTER                      0x1A0 //R  
#define APIC_TRIGGER_MODE3_REGISTER                      0x1B0 //R  
#define APIC_TRIGGER_MODE4_REGISTER                      0x1C0 //R  
#define APIC_TRIGGER_MODE5_REGISTER                      0x1D0 //R  
#define APIC_TRIGGER_MODE6_REGISTER                      0x1E0 //R  
#define APIC_TRIGGER_MODE7_REGISTER                      0x1F0 //R  
#define APIC_INTERRUPT_REQUEST0_REGISTER                 0x200 //R  
#define APIC_INTERRUPT_REQUEST1_REGISTER                 0x210 //R  
#define APIC_INTERRUPT_REQUEST2_REGISTER                 0x220 //R  
#define APIC_INTERRUPT_REQUEST3_REGISTER                 0x230 //R  
#define APIC_INTERRUPT_REQUEST4_REGISTER                 0x240 //R  
#define APIC_INTERRUPT_REQUEST5_REGISTER                 0x250 //R  
#define APIC_INTERRUPT_REQUEST6_REGISTER                 0x260 //R  
#define APIC_INTERRUPT_REQUEST7_REGISTER                 0x270 //R  
#define APIC_ERROR_STATUS_REGISTER                       0x280 //R  
#define APIC_LVT_CMCI_REGISTER                           0x2F0 //R W
#define APIC_INTERRUPT_COMMAND0_REGISTER                 0x300 //R W
#define APIC_INTERRUPT_COMMAND1_REGISTER                 0x310 //R W
#define APIC_LVT_TIMER_REGISTER                          0x320 //R W
#define APIC_LVT_THERMAL_SENSOR_REGISTER                 0x330 //R W
#define APIC_LVT_PERFORMANCE_MONITORING_COUNTER_REGISTER 0x340 //R W
#define APIC_LVT_LINT0_REGISTER                          0x350 //R W
#define APIC_LVT_LINT1_REGISTER                          0x360 //R W
#define APIC_ERROR_REGISTER                              0x370 //R W
#define APIC_INITIAL_COUNT_REGISTER                      0x380 //R W
#define APIC_CURRENT_COUNT_REGISTER                      0x390 //R
#define APIC_DIVIDE_CONFIGURATION_REGISTER               0x3E0 //R W

void
apic_enable(uint32 apic_base_address);

void
apic_write_reg(uint32 reg, uint32 val);

uint32
apic_read_reg(uint32 reg);

#endif
