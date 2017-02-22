CC=gcc
CFLAGS=-m32 -O3 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -std=gnu99 -ffreestanding -ggdb
CINCLUDES= -I ./

LDFLAGS=-T link.ld -melf_i386 

AS=nasm
ASFLAGS=-f elf

SUBDIRS=
OBJS= \
	 memory.o string.o list.o

all: $(SUBDIRS) $(OBJS)
	cp *.o $(BUILD_ROOT)/OBJS

$(SUBDIRS):
	$(MAKE) -C $@ BUILD_ROOT="$(BUILD_ROOT)"

%.o: %.c
	$(CC) $(CFLAGS) $(CINCLUDES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o