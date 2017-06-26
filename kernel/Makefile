CINCLUDES=-I ./ -I ../libc/

SUBDIRS="arch/x86"
SUBDIRSCLEAN=$(addsuffix .clean, $(SUBDIRS))
SUBDIRSBUILD=$(addsuffix .build, $(SUBDIRS))

OBJS= \
	 time.o timer.o keyboard.o kmain.o __start.o

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