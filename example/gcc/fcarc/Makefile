GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
OBJCOPY=tms9900-objcopy
FCSDK=$(abspath ../fcsdk)
XDM99?=$(shell which xdm99.py)

FNAME=FCARC

LDFLAGS=--script=$(FCSDK)/fc.ld --script=linkfile
CFLAGS=-std=gnu99 -O2 -Werror --save-temps -I$(FCSDK)

C_SRCS=$(wildcard *.c)
ASM_SRCS=$(wildcard *.asm)
OBJECT_LIST=$(sort $(C_SRCS:.c=.o) $(ASM_SRCS:.asm=.o))
LINK_OBJECTS=$(addprefix objects/,$(OBJECT_LIST))

BANK_ORDER=objects/bank0.page objects/bank1.page objects/bank2.page objects/bank3.page objects/bank4.page objects/bank5.page objects/bank6.page objects/bank7.page
BIN_ORDER=$(BANK_ORDER) objects/data.bin

all: $(FNAME)

$(FNAME).elf: $(LINK_OBJECTS)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $@ -Map=mapfile

$(FNAME).bin: $(BIN_ORDER)
	cat $(BIN_ORDER) > $@

objects/data.bin: $(FNAME).elf
	$(OBJCOPY) -O binary -j .data $< objects/data.bin
	# grow the data page to a round 4k multiple
	DATASIZE=`stat --printf="%s" objects/data.bin`; \
	DATASIZE=`expr $$DATASIZE / 4096`; \
	DATASIZE=`expr 1 + $$DATASIZE`; \
	DATASIZE=`expr 4096 \* $$DATASIZE`; \
	dd if=/dev/null of=$@ bs=$$DATASIZE seek=1

objects/%.page: $(FNAME).elf
	$(OBJCOPY) -O binary -j .$(shell basename $@ .page) $< $@
	# grow each bank to 8k
	@dd if=/dev/null of=$@ bs=8192 seek=1

$(FNAME): $(FNAME).bin
	python3 $(XDM99) -T $< -t -f PROGRAM -n $(FNAME) -o $(FNAME)
	mkdir -p ../../../FC/BIN && cp $(FNAME) ../../../FC/BIN/

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) $(abspath $<) -o $(notdir $@)

define bankdefs
	-DSAMS_CURRENT_BANK=$(shell echo $(basename $1) | cut -c2)
endef

objects/%.o: %.c $(wildcard *.h)
	mkdir -p objects; cd objects; $(CC) -c $(abspath $<) $(CFLAGS) $(call bankdefs,$<) -o $(notdir $@)

clean:
	rm -fr objects
	rm -f *.elf *.bin mapfile $(FNAME)
