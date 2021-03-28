GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump
XGA99?=$(shell which xga99.py)

FNAME=FCMD
UCFNAME=$(shell echo -n $(FNAME) | tr 'a-z' 'A-Z')

BANKBINS:=$(shell seq -s ' ' -f "bank%1g.page" 0 15)

LDFLAGS=\
  --script=linkfile

CFLAGS=\
  -std=gnu99 -O2 -Werror --save-temp -I.. -I../libti99 -DBANK_STACK_SIZE=15

SRCS:=$(sort $(wildcard *.c) $(wildcard *.asm))
LIBTI99_SRCS=$(sort $(wildcard libti99/*.c))

LIBTI99_OBJS:=$(notdir $(LIBTI99_SRCS:.c=.o))
OBJECT_LIST:=$(SRCS:.c=.o) $(LIBTI99_OBJS)
OBJECT_LIST:=$(filter-out api.o b3_fcbanner.o,$(OBJECT_LIST:.asm=.o)) api.o b3_fcbanner.o

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: $(FNAME)G.bin $(FNAME)8.bin subdirs

# The size of the cart_rom segment in decimal
# must agree with linkfile
COMMON_SIZE = 128

HEADBIN:=objects/header.bin

$(HEADBIN): $(FNAME).elf
	$(OBJCOPY) -O binary -j .text $< $(HEADBIN)
	@dd if=/dev/null of=$(HEADBIN) bs=$(COMMON_SIZE) seek=1

bank0.page: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .data $< objects/data.bin_tmp
	$(OBJCOPY) -O binary -j .bank0 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp objects/data.bin_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

%.page: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .$(basename $@) $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

$(FNAME)8.bin: $(BANKBINS)
	cat $^ >$@

$(FNAME)G.bin: gpl-boot.g99 $(FNAME).elf
	python3 $(XGA99) -D "CART=$(shell echo -n '>' ; grep _cart mapfile | sed 's/^\s*0x0*\([0-9a-f]*\) *_cart/\1/')" -o $@ $<

$(FNAME).elf: $(LINK_OBJECTS) linkfile
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

linkfile: linkfile.m4
	m4 $< > $@

clean:
	rm -fr objects
	rm -f *.elf
	rm -f *.bin
	rm -f *.page
	rm -f linkfile
	rm -f mapfile
	rm -f *.RPK
	rm -f api.asm
	rm -f api.banks
	for d in $(SUBDIRS); do $(MAKE) -C example/gcc/$$d clean; done

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) ../$< -o $(notdir $@)

objects/%.o: %.c
	mkdir -p objects; cd objects; $(CC) -c ../$< $(CFLAGS) -o $(notdir $@)

objects/%.o: libti99/%.c
	mkdir -p objects; cd objects; $(CC) -c ../$< $(CFLAGS) -o $(notdir $@)

api.asm: api.lst makeapi.py fc_api_template
	rm -f api.asm
	grep DECLARE_BANKED *.h >api.banks
	python3 ./makeapi.py api.lst api.asm api.banks example/gcc/fc_api.h

b3_fcbanner.asm: fcbanner.ans ans2asm.py
	python3 ./ans2asm.py

SUBDIRS=hello ntscpal charset ftp say

subdirs: api.asm
	for d in $(SUBDIRS); do $(MAKE) -C example/gcc/$$d; done

.PHONY: clean subdirs

