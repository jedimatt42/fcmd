GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
LIBTI99?=/home/matthew/dev/github/jedimatt42/libti99
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump
XGA99?=$(shell which xga99.py)

FNAME=FCMD
UCFNAME=$(shell echo -n $(FNAME) | tr 'a-z' 'A-Z')

LDFLAGS=\
  --script=linkfile -L$(LIBTI99) -lti99

CFLAGS=\
  -std=gnu99 -O2 -Werror --save-temp -I$(LIBTI99) -DBANK_STACK_SIZE=10

SRCS:=$(sort $(wildcard *.c) $(wildcard *.asm))

OBJECT_LIST:=$(SRCS:.c=.o)
OBJECT_LIST:=$(OBJECT_LIST:.asm=.o)

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: $(FNAME)C.bin $(FNAME)G.bin

# The size of the cart_rom segment in decimal
# must agree with linkfile
COMMON_SIZE = 112

HEADBIN:=objects/header.bin

$(HEADBIN): $(FNAME).elf
	$(OBJCOPY) -O binary -j .text $< $(HEADBIN)
	@dd if=/dev/null of=$(HEADBIN) bs=$(COMMON_SIZE) seek=1

objects/libti99_tmp: $(FNAME).elf
	$(OBJCOPY) -O binary -j .libti99 $< $@

bank0.bin: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .data $< objects/data.bin_tmp
	$(OBJCOPY) -O binary -j .bank0 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp objects/data.bin_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank1.bin: $(FNAME).elf $(HEADBIN) objects/libti99_tmp
	$(OBJCOPY) -O binary -j .bank1 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp objects/libti99_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank2.bin: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .bank2 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank3.bin: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .bank3 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank4.bin: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .bank4 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank5.bin: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .bank5 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank6.bin: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .bank6 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank7.bin: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .bank7 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

$(FNAME)C.bin: bank0.bin bank1.bin bank2.bin bank3.bin bank4.bin bank5.bin bank6.bin bank7.bin
	cat $^ >$@

$(FNAME)G.bin: gpl-boot.g99 $(FNAME).elf
	python2 $(XGA99) -D "CART=$(shell echo -n '>' ; grep _cart mapfile | sed 's/^\s*0x0*\([0-9a-f]*\) *_cart/\1/')" -o $@ $<

$(FNAME).elf: $(OBJECT_LIST)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

.phony clean:
	rm -fr objects
	rm -f *.elf
	rm -f *.bin
	rm -f mapfile

%.o: %.asm
	mkdir -p objects
	cd objects; $(GAS) ../$< -o $@

%.o: %.c
	mkdir -p objects
	cd objects; $(CC) -c ../$< $(CFLAGS) -I/home/matthew/dev/gcc-9900/lib/gcc/tms9900/4.4.0/include -o $@

