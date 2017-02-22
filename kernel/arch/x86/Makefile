CC=gcc
CFLAGS=-m32 -O3 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -std=gnu99 -ffreestanding -ggdb
CINCLUDES=-I ../../ -I ../../../libc/

LDFLAGS=-T link.ld -melf_i386 

AS=nasm
ASFLAGS=-f elf

OBJS= \
		framebuffer.o gdt.o idt.o io.o irq.o page.o int.o COM1.o cpuid.o msr.o apic.o acpi.o pit.o

all: $(OBJS)
	cp *.o $(BUILD_ROOT)/OBJS

%.o: %.c
	$(CC) $(CFLAGS) $(CINCLUDES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o