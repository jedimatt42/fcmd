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

C_SRCS:=$(wildcard *.c)
ASM_SRCS:=$(wildcard *.asm) 

OBJECT_LIST:=$(sort $(C_SRCS:.c=.o) $(ASM_SRCS:.asm=.o))

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))

all: $(FNAME).bin

# The size of the cart_rom segment in decimal
# must agree with linkfile
COMMON_SIZE = 128

$(FNAME).elf: $(LINK_OBJECTS)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

$(FNAME).bin: $(FNAME).elf
	$(OBJCOPY) -O binary $< $@

.phony clean:
	rm -fr objects
	rm -f *.elf *.bin mapfile

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) ../$< -o $(notdir $@)

objects/%.o: %.c
	mkdir -p objects; cd objects; $(CC) -c ../$< $(CFLAGS) -o $(notdir $@)

