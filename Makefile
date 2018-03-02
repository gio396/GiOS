CC=gcc
GDB=gdb

CFLAGS= -m32 -O0 -ggdb -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -std=gnu99 -ffreestanding -Wno-unused-parameter
BASEDIR=$(shell pwd)
CINCLUDES=-I $(BASEDIR)/kernel/  -I $(BASEDIR)/libc/
DEFINES=-D__GIOS_DEBUG__

LDFLAGS=-T link.ld -melf_i386 

export CFLAGS
export CINCLUDES
export DEFINES

export CFLAGS
export CC

OBJS=$(.o)
AS=nasm
ASFLAGS=-f elf

export AS
export ASFLAGS

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

QEMU_COMMON_FLAGS=-m 256 -cpu core2duo -smp 1 -device virtio-serial,id=nxtoolsBus0 -device virtserialport,chardev=nxtoolsChardev0,name=nxtools0,id=nxtoolsGuest0 -chardev socket,path=/tmp/GOV,server,id=nxtoolsChardev0

#--drive file=image_file.raw,if=virtio

#FOR AHCI device use this!
#-drive id=disk,file=image_file.raw,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0

run: clean dir os.iso
		qemu-system-x86_64 -boot d -kernel kernel.elf $(QEMU_COMMON_FLAGS) -serial stdio

qemu_dbg: clean dir os.iso
		qemu-system-x86_64 -boot d -kernel kernel.elf $(QEMU_COMMON_FLAGS) -monitor stdio

qemu_dbg_s: clean dir os.iso
		qemu-system-x86_64 -boot d -kernel kernel.elf $(QEMU_COMMON_FLAGS) -monitor stdio -S


gdb:
	$(GDB) --quiet kernel.sym -ex "target remote 127.0.0.1:1234"

dir:
	mkdir -p OBJS

$(SUBDIRSBUILD):
	$(MAKE) -C $(basename $@) BUILD_ROOT="$(BUILD_ROOT)"

%.o: %.c
	$(CC) $(CFLAGS) $(CINCLUDES) $(DEFINES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean: $(SUBDIRSCLEAN)
	rm -rf OBJS
	rm -rf *.o kernel.elf os.iso

$(SUBDIRSCLEAN):
	$(MAKE) -C $(basename $@) clean