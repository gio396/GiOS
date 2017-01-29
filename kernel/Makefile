CC=gcc
CFLAGS=-m32 -O0 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -std=gnu99 -ffreestanding -ggdb
CINCLUDES=-I ./ -I ../libc/

LDFLAGS=-T link.ld -melf_i386 

AS=nasm
ASFLAGS=-f elf

SUBDIRS="arch/x86"
SUBDIRSCLEAN=$(addsuffix .clean, $(SUBDIRS))
SUBDIRSBUILD=$(addsuffix .build, $(SUBDIRS))

OBJS= \
	 keyboard.o kmain.o __start.o

all: $(SUBDIRSBUILD) $(OBJS)
	cp *.o $(BUILD_ROOT)/OBJS

$(SUBDIRSBUILD):
	$(MAKE) -C $(basename $@) BUILD_ROOT="$(BUILD_ROOT)"

%.o: %.c
	$(CC) $(CFLAGS) $(CINCLUDES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean: $(SUBDIRSCLEAN)
	rm -rf *.o

$(SUBDIRSCLEAN):
	$(MAKE) -C $(basename $@) clean