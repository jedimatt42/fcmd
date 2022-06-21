GAS=$(shell which tms9900-as)
LD=$(shell which tms9900-ld)
CC=$(shell which tms9900-gcc)
OBJCOPY=$(shell which tms9900-objcopy)
OBJDUMP=$(shell which tms9900-objdump)
XGA99?=$(shell which xga99.py)
XDM99?=$(shell which xdm99.py)

FNAME=FCMD

BANKBINS:=$(shell seq -s ' ' -f "bank%1g.page" 0 15)

VER:=$(shell grep "\#define APP_VER" b0_main.h | cut -d '"' -f2)

MANIFEST=FCMDG.bin FCMDC.bin README.md
SUPPORT=FC/LOAD FC/FCMD FC/FCMDXB FC/BIN/FCMENU FC/BIN/FTP FC/BIN/SAMPLE FC/BIN/SAY FC/BIN/TELNET FC/BIN/VIRGIL99 FC/BIN/FONT FC/MDOSANSI FC/MDOSFONT

LDFLAGS=\
  --script=linkfile

CFLAGS=\
  -std=gnu99 -nostdlib -ffreestanding -O2 -Werror --save-temps -I$(abspath .) -I$(abspath libti99)

SRCS:=$(sort $(wildcard *.c) $(wildcard *.asm))
LIBTI99_SRCS=$(sort $(wildcard libti99/*.c))

LIBTI99_OBJS:=$(notdir $(LIBTI99_SRCS:.c=.o))
OBJECT_LIST:=$(SRCS:.c=.o) $(LIBTI99_OBJS)
OBJECT_LIST:=$(filter-out api.o b3_fcbanner.o,$(OBJECT_LIST:.asm=.o)) api.o b3_fcbanner.o

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: forcecmd_$(VER).zip

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

$(FNAME)C.bin: $(BANKBINS)
	cat $^ >$@

$(FNAME)G.bin: gpl-boot.g99 $(FNAME).elf
	python3 $(XGA99) -D "CART=$(shell echo -n '>' ; grep _cart mapfile | sed 's/^\s*0x0*\([0-9a-f]*\) *_cart/\1/')" -o $@ $<

$(FNAME).elf: $(LINK_OBJECTS) linkfile
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

linkfile: linkfile.m4
	m4 $< > $@

clean:
	rm -f forcecmd_*.zip
	rm -fr objects
	rm -f *.elf
	rm -f *.bin
	rm -f *.page
	rm -f linkfile
	rm -f mapfile
	rm -f *.RPK
	rm -f api.asm
	rm -f api.banks
	$(MAKE) -C FC clean
	for d in $(SUBDIRS); do $(MAKE) -C example/gcc/$$d clean; done
	rm -fr fcsdk

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) $(abspath $<) -o $(notdir $@)

objects/%.o: %.c
	mkdir -p objects; cd objects; $(CC) -c $(abspath $<) $(CFLAGS) -o $(notdir $@)

objects/%.o: libti99/%.c
	mkdir -p objects; cd objects; $(CC) -c $(abspath $<) $(CFLAGS) -o $(notdir $@)

api.asm: fc_api.lst makeapi.py fc_api_template
	rm -f api.asm
	grep DECLARE_BANKED libti99/*.h *.h >api.banks
	python3 ./makeapi.py fc_api.lst api.asm api.banks example/gcc/fcsdk/fc_api.h

b3_fcbanner.asm: fcbanner.ans ans2asm.py
	python3 ./ans2asm.py

SUBDIRS=hello samshello charset fcmenu ftp say telnet virgil font

subdirs: api.asm
	for d in $(SUBDIRS); do $(MAKE) -C example/gcc/$$d; done

support: FC/reload_fcmd.asm FC/loadxb.bas
	$(MAKE) -C FC

$(FNAME).DSK: subdirs support
	python3 $(XDM99) $(FNAME).DSK -X 360 -t -a FC/BIN/FCMENU FC/BIN/FTP FC/BIN/TELNET FC/BIN/SAY FC/BIN/SAMPLE FC/BIN/VIRGIL99 FC/BIN/FONT FC/MDOSANSI FC/MDOSFONT

$(FNAME).RPK: $(FNAME)C.bin $(FNAME)G.bin layout.xml
	zip $@ $^

$(FNAME)_ALT.RPK: $(FNAME)C.bin layout_alt.xml
	cp $^ /tmp
	mv /tmp/layout_alt.xml /tmp/layout.xml
	cd /tmp && zip $@ layout.xml $(FNAME)C.bin
	mv /tmp/$@ $@

fcsdk:
	rm -f ./fcsdk
	cp -a example/gcc/fcsdk ./fcsdk
	cp -a example/gcc/hello ./fcsdk/hello
	cp -a example/gcc/samshello ./fcsdk/samshello
	find fcsdk -name "objects" -o -name "mapfile" -o -name "*HELLO*" | xargs rm -r

forcecmd_$(VER).zip: $(MANIFEST) subdirs support $(FNAME).RPK $(FNAME).DSK fcsdk
	rm -f $@
	zip -r $@ $(MANIFEST) $(SUPPORT) $(FNAME).RPK $(FNAME).DSK fcsdk

.PHONY: clean subdirs support

