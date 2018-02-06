OBJS= \
	device.o device_bus.o

all: $(SUBDIRS) $(OBJS)
	cp *.o $(BUILD_ROOT)/OBJS

$(SUBDIRS):
	$(MAKE) -C $@ BUILD_ROOT="$(BUILD_ROOT)"

%.o: %.c
	$(CC) $(CFLAGS) $(CINCLUDES) $(DEFINES)  $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o