CINCLUDES= -I ./

SUBDIRS=
OBJS= \
	string.o list.o

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