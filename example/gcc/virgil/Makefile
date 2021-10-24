GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump
XDM99?=$(shell which xdm99.py)

FNAME=VIRGIL99

LDFLAGS=\
  --script=linkfile

CFLAGS=\
  -std=gnu99 -O2 -Werror --save-temp -I../..

C_SRCS:=$(wildcard *.c)
ASM_SRCS:=$(wildcard *.asm) 

OBJECT_LIST:=$(sort $(C_SRCS:.c=.o) $(ASM_SRCS:.asm=.o))

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: $(FNAME)

$(FNAME).elf: $(LINK_OBJECTS)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

$(FNAME).bin: $(FNAME).elf
	$(OBJCOPY) -O binary $< $@

$(FNAME): $(FNAME).bin
	python3 $(XDM99) -T $< -t -f PROGRAM -n $(FNAME) -o $(FNAME)
	mkdir -p ../../../FC/BIN && cp $(FNAME) ../../../FC/BIN/

.phony clean:
	rm -fr objects
	rm -f *.elf *.bin mapfile
	rm -f $(FNAME)

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) $(abspath $<) -o $(notdir $@)

objects/%.o: %.c ../fc_api.h
	mkdir -p objects; cd objects; $(CC) -c $(abspath $<) $(CFLAGS) -o $(notdir $@)

