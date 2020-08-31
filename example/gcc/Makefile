GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump

FNAME=HELLO
UCFNAME=$(shell echo -n $(FNAME) | tr 'a-z' 'A-Z')

LDFLAGS=\
  --script=linkfile

CFLAGS=\
  -std=gnu99 -O2 -Werror --save-temp -I.. 

SRCS:=$(sort $(wildcard *.c) $(wildcard *.asm)) 

OBJECT_LIST:=$(SRCS:.c=.o)

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: $(FNAME).elf

# The size of the cart_rom segment in decimal
# must agree with linkfile
COMMON_SIZE = 128

$(FNAME).elf: $(LINK_OBJECTS)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile
	if grep 0xfffff mapfile | grep -q __STATS_BANK; then echo "FAIL: Bank Overflow"; false; fi

.phony clean:
	rm -fr objects
	rm -f *.elf
	rm -f mapfile
	rm -f *.RPK

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) ../$< -o $(notdir $@)

objects/%.o: %.c
	mkdir -p objects; cd objects; $(CC) -c ../$< $(CFLAGS) -o $(notdir $@)

