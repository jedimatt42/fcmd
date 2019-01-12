GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
LIBTI99?=/home/matthew/dev/github/jedimatt42/libti99
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump

FNAME=tipicmd
UCFNAME=$(shell echo -n $(FNAME) | tr 'a-z' 'A-Z')

LDFLAGS=\
  --script=linkfile -L$(LIBTI99) -lti99

CFLAGS=\
  -std=gnu99 -O2 --save-temp -I$(LIBTI99) -DBANK_STACK_SIZE=5

SRCS:=$(sort $(wildcard *.c) $(wildcard *.asm))

OBJECT_LIST:=$(SRCS:.c=.o)
OBJECT_LIST:=$(OBJECT_LIST:.asm=.o)

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: $(FNAME)_8.bin

# The size of the cart_rom segment in decimal
# must agree with linkfile
COMMON_SIZE = 112

header.bin: $(FNAME).elf
	$(OBJCOPY) -O binary -j .text $< header.bin
	@dd if=/dev/null of=header.bin bs=$(COMMON_SIZE) seek=1

bank0.bin: $(FNAME).elf header.bin
	$(OBJCOPY) -O binary -j .data $< data_tmp.bin
	$(OBJCOPY) -O binary -j .bank0 $< $@_tmp
	cat header.bin $@_tmp data_tmp.bin >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank1.bin: $(FNAME).elf header.bin
	$(OBJCOPY) -O binary -j .bank1 $< $@_tmp
	cat header.bin $@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank2.bin: $(FNAME).elf header.bin
	$(OBJCOPY) -O binary -j .bank2 $< $@_tmp
	cat header.bin $@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank3.bin: $(FNAME).elf header.bin
	$(OBJCOPY) -O binary -j .bank3 $< $@_tmp
	cat header.bin $@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

$(FNAME)_8.bin: bank0.bin bank1.bin bank2.bin bank3.bin
	cat $^ >$@

$(FNAME).elf: $(OBJECT_LIST)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

.phony clean:
	rm -fr objects
	rm -f *.elf
	rm -f *.bin
	rm -f *.bin_tmp
	rm -f mapfile

%.o: %.asm
	mkdir -p objects
	cd objects; $(GAS) ../$< -o $@

%.o: %.c
	mkdir -p objects
	cd objects; $(CC) -c ../$< $(CFLAGS) -I/home/matthew/dev/gcc-9900/lib/gcc/tms9900/4.4.0/include -o $@

