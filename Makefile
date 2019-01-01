GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
EA5SPLIT=ea5split
LIBTI99?=/home/matthew/dev/github/jedimatt42/libti99
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump

FNAME=banktest
UCFNAME=$(shell echo -n $(FNAME) | tr 'a-z' 'A-Z')

LDFLAGS=\
  --script=linkfile

CFLAGS=\
  -std=c99 -O2 --save-temp

SRCS:=$(sort $(wildcard *.c) $(wildcard *.asm))

OBJECT_LIST:=$(SRCS:.c=.o)
OBJECT_LIST:=$(OBJECT_LIST:.asm=.o)

all: $(FNAME)_8.bin

header.bin: $(FNAME).elf
	$(OBJCOPY) -O binary -j .text $^ header.bin
	ls -l header.bin
	@dd if=/dev/null of=header.bin bs=256 seek=1

bank0.bin: $(FNAME).elf header.bin
	$(OBJCOPY) -O binary -j .bank0 $< bank0_tmp.bin
	$(OBJCOPY) -O binary -j .data $< data_tmp.bin
	cat header.bin bank0_tmp.bin data_tmp.bin >bank0.bin
	@dd if=/dev/null of=bank0.bin bs=8192 seek=1

bank1.bin: $(FNAME).elf header.bin
	$(OBJCOPY) -O binary -j .bank1 $< bank1_tmp.bin
	cat header.bin bank1_tmp.bin >bank1.bin
	@dd if=/dev/null of=bank1.bin bs=8192 seek=1

$(FNAME)_8.bin: bank0.bin bank1.bin
	cat bank0.bin bank1.bin >$@

$(FNAME).elf: $(OBJECT_LIST)
	$(LD) $(OBJECT_LIST) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

.phony clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.bin
	rm -f *.i
	rm -f *.s
	rm -f mapfile

%.o: %.asm
	$(GAS) $< -o $@

%.o: %.c
	$(CC) -c $< $(CFLAGS) -I$(LIBTI99) -I/home/matthew/dev/gcc-9900/lib/gcc/tms9900/4.4.0/include -o $@

