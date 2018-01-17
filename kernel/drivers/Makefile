SUBDIRS="virtio"
SUBDIRSCLEAN=$(addsuffix .clean, $(SUBDIRS))
SUBDIRSBUILD=$(addsuffix .build, $(SUBDIRS))

OBJS= \

all: $(SUBDIRSBUILD) $(OBJS)

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