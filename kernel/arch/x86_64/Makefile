CINCLUDES=-I ../../ -I ../../../libc/

CFLAGS= -m64 -O0 -ggdb -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c -std=gnu99 -ffreestanding

OBJS= 

all: $(OBJS)
	cp *.o $(BUILD_ROOT)/OBJS

%.o: %.c
	$(CC) $(CFLAGS) $(CINCLUDES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o