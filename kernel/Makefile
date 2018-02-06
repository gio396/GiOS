SUBDIRS="arch/x86" "drivers"
SUBDIRSCLEAN=$(addsuffix .clean, $(SUBDIRS))
SUBDIRSBUILD=$(addsuffix .build, $(SUBDIRS))

OBJS= \
	 kinit.o memory.o time.o timer.o keyboard.o kmain.o __start.o

all: $(SUBDIRSBUILD) $(OBJS)
	cp *.o $(BUILD_ROOT)/OBJS

$(SUBDIRSBUILD):
	$(MAKE) -C $(basename $@) BUILD_ROOT="$(BUILD_ROOT)"

%.o: %.c
	$(CC) $(CFLAGS) $(CINCLUDES) $(DEFINES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean: $(SUBDIRSCLEAN)
	rm -rf *.o

$(SUBDIRSCLEAN):
	$(MAKE) -C $(basename $@) clean