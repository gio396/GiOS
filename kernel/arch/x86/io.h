#ifndef __IO_H__
#define __IO_H__

#include <common.h>

#define KB_COMMAND_PORT 0x60
#define KB_DATA_PORT    0x64
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PROT    0x3D5

  
u32 
outb(u16 port, u16 data);

u32
outl(u16 port, u32 data);

u32
outs(u16 port, u32 data);

u32 
inb(u16 port);

u32
ins(u16 port);

u32
inl(u16 port);

u32 
halt();

void
nop(void);

i32 
idt_load(u32 idt_ptr);

i32 
gdt_flush(u32 gdt_ptr);

void 
init_serial();

void 
write_serial(u8 a);

#endif