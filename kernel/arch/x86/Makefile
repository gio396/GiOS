CINCLUDES=-I ../../ -I ../../../libc/

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