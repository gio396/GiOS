#ifndef __IO_H__
#define __IO_H__

#include <common.h>

#define KB_COMMAND_PORT 0x60
#define KB_DATA_PORT    0x64
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PROT    0x3D5

  
int32 outb(uint16 port, uint16 data);

int32 inb(uint16 port);

int32 halt();

int32 idt_load(uint32 idt_ptr);

int32 gdt_flush(uint32 gdt_ptr);

#endif