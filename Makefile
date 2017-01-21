CC=gcc
CFLAGS=-m32 -O3 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -std=gnu99 -ffreestanding -ggdb
CINCLUDES=-I kernel/  -I libc/

LDFLAGS=-T link.ld -melf_i386 

OBJS=$(.o)
AS=nasm
ASFLAGS=-f elf

SUBDIRS:=kernel libc
SUBDIRSCLEAN:=$(addsuffix  .clean ,$(SUBDIRS))
SUBDIRSBUILD:=$(addsuffix  .build ,$(SUBDIRS))

BUILD_ROOT:=$(shell pwd)

.PHONY all: dir kernel.elf

.PHONY again: clean run

kernel.elf: $(SUBDIRSBUILD)
	ld $(LDFLAGS) OBJS/* -o kernel.elf

os.iso: kernel.elf
	objcopy --only-keep-debug kernel.elf kernel.sym
	objcopy --strip-debug kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	# genisoimage -R \
	# 						-b boot/grub/stage2_eltorito    \
	# 						-no-emul-boot                   \
	# 						-boot-load-size 4               \
	# 						-A os                           \
	# 						-input-charset utf8             \
	# 						-quiet                          \
	# 						-boot-info-table                \
	# 						-o os.iso                       \
	# 						iso

run: dir os.iso
		qemu-system-x86_64  -boot d -kernel kernel.elf -m 4096  -monitor stdio

dir:
	mkdir -p OBJS

$(SUBDIRSBUILD):
	$(MAKE) -C $(basename $@) BUILD_ROOT="$(BUILD_ROOT)"

%.o: %.c
	$(CC) $(CFLAGS) (CINCLUDES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean: $(SUBDIRSCLEAN)
	rm -rf OBJS
	rm -rf *.o kernel.elf os.iso

$(SUBDIRSCLEAN):
	$(MAKE) -C $(basename $@) clean