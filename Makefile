GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
LIBTI99?=/home/matthew/dev/github/jedimatt42/libti99
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump
XGA99?=$(shell which xga99.py)

FNAME=FCMD
UCFNAME=$(shell echo -n $(FNAME) | tr 'a-z' 'A-Z')

BANKBINS:=$(shell seq -s ' ' -f "bank%1g.page" 0 15)

LDFLAGS=\
  --script=linkfile -L$(LIBTI99) -lti99

CFLAGS=\
  -std=gnu99 -O2 -Werror --save-temp -I$(LIBTI99) -DBANK_STACK_SIZE=15

SRCS:=$(sort $(wildcard *.c) $(wildcard *.asm))

OBJECT_LIST:=$(SRCS:.c=.o)
OBJECT_LIST:=$(filter-out api.o b3_fcbanner.o,$(OBJECT_LIST:.asm=.o)) api.o b3_fcbanner.o

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: $(FNAME)C.bin $(FNAME)G.bin

# The size of the cart_rom segment in decimal
# must agree with linkfile
COMMON_SIZE = 128

HEADBIN:=objects/header.bin

$(HEADBIN): $(FNAME).elf
	$(OBJCOPY) -O binary -j .text $< $(HEADBIN)
	@dd if=/dev/null of=$(HEADBIN) bs=$(COMMON_SIZE) seek=1

objects/libti99_tmp: $(FNAME).elf
	$(OBJCOPY) -O binary -j .libti99 $< $@

bank0.page: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .data $< objects/data.bin_tmp
	$(OBJCOPY) -O binary -j .bank0 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp objects/data.bin_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

bank1.page: $(FNAME).elf $(HEADBIN) objects/libti99_tmp
	$(OBJCOPY) -O binary -j .bank1 $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp objects/libti99_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

%.page: $(FNAME).elf $(HEADBIN)
	$(OBJCOPY) -O binary -j .$(basename $@) $< objects/$@_tmp
	cat $(HEADBIN) objects/$@_tmp >$@
	@dd if=/dev/null of=$@ bs=8192 seek=1

$(FNAME)C.bin: $(BANKBINS)
	cat $^ >$@

$(FNAME)G.bin: gpl-boot.g99 $(FNAME).elf
	python3 $(XGA99) -D "CART=$(shell echo -n '>' ; grep _cart mapfile | sed 's/^\s*0x0*\([0-9a-f]*\) *_cart/\1/')" -o $@ $<

$(FNAME).elf: $(LINK_OBJECTS)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile
	if grep 0xfffff mapfile | grep -q __STATS_BANK; then echo "FAIL: Bank Overflow"; false; fi

.phony clean:
	rm -fr objects
	rm -f *.elf
	rm -f *.bin
	rm -f mapfile
	rm -f *.RPK
	rm -f api.asm
	rm -f api.banks

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) ../$< -o $(notdir $@)

objects/%.o: %.c
	mkdir -p objects; cd objects; $(CC) -c ../$< $(CFLAGS) -o $(notdir $@)

api.asm: api.lst makeapi.py
	rm -f api.asm
	for f in `cat api.lst`; do grep $$f b*.h; done | grep BANK_ | cut -d'(' -f2 | cut -d',' -f1-2 >api.banks
	python2 makeapi.py api.lst api.banks api.asm

b3_fcbanner.asm: fcbanner.ans ans2asm.py
	python ./ans2asm.py

