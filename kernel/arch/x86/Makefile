CINCLUDES=-I ../../ -I ../../../libc/

CFLAGS=-m32 -O0 -ggdb -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -std=gnu99 -ffreestanding

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