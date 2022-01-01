# SAMSHELLO banked code example

Everything is much like the description in [hello](../hello), with the addition of declaring and linking into SAMS banked memory.

The Force Command mechanism isn't trying to be all for everyone. There are infinate ways to organize usage of sams memory. Many of us are familiar with paging code in ROMs using large cartridge formats. I've abstracted code banking in SAMS to use the same-ish mechanism I use in cartridge ROMs. The first 8k of the upper memory expansion is the code space, where all the .text segment of the binary is loaded. Banks are numbered relative to the base page available when your program is loaded. Bank numbers represent 8k chunks. Your program doesn't need to worry about 4K boundaries.

- _header.asm
- alt.h
- b0_hello.c
- b1_alt.c
- linkfile
- Makefile

## Declaring pages

```asm
    REF fc_main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0004   ; SAMS page count - 0x0004 means sams required, 2 banks of 2 pages - 4
    data 0xFCFC   ; Flag that program promises to use screen safely
    data fc_main     ; program start address
```

The only difference here is the second word of the header is now 0x0004 to indicate that we are to be loaded into
paged memory as 4 pages, 2 8k banks. So that gives us space for bank 0 and bank 1.

The _header.o will end up in bank 0 when we link. And fc_main must be in bank 0.

## Source code files

We have a function in b1_alt.c that we want to access via bank switching from any bank. So we declare it using the FC_SAMS_BANKED or in this case, FC_SAMS_VOIDBANKED (because the return type is void). 

FC_SAMS_VOIDBANKED requires the following parameters:

- bank_id - the 8k bank number the function resides in.
- function - the symbolic name of the function that will be declared.
- signature - the function prototype signature, in this no params so just ()
- call params - the parm list for calling. This is like the signature without the types.

```c
#pragma once

#include <fc_api.h>

FC_SAMS_VOIDBANKED(1, alt, (), ());
```

That will declare an inline function that handles all the bank switching logic:

```c
void alt();
```

Our .c source files are organized with a bank prefix, so that the linker can automatically pack them into the correct pages. b0_*.c goes in bank 0, and b1_*.c go in bank 1. 

b0_hello.c looks pretty normal... it includes the alt.h header file to get the signature of the alt() function, and can just call it. 

```c
#include <fc_api.h>

#include "alt.h"

int fc_main(char* args) {
  fc_tputs("Hello\n");
  alt();
  return 0;
}
```

b1_alt.c is defining the alt() function using the FC_SAMS macro. This actually defines b0_alt(), which you'll see in the generated mapfile. Callers of the function in the same bank, or in other banks should still use the declaration of alt() so the code is natural. 

```c
#include <fc_api.h>

void FC_SAMS(1, alt()) {
  fc_tputs("alt Hello\n");
  return;
}
```

The bank switching uses the stack, so you can go in and out, and around through banks without worrying. The only thing you have to be cautious of, is static data such as const character strings that end up in your banked .text segments. They cannot be given directly as arguments to banked functions as they will get banked out with the calling code. This can be mitigated by either making them not const so they end up in the .data segment, or any number of creative means using the remaining 16k of RAM not used by banking, or data on the gcc stack which lives in the lower memory expansion.

## Linking

The linkfile is more complex. Instead of one memory region to declare, we have 2. The first `samsbanked` is the 8k region that each bank will be packed into. The second is our remaining ram starting at 0xC000 where our .data and .bss segments will go.

This time when linking the .text segments, we create .bank0 and .bank1 as sections that OVERLAY each other in the `samsbanked` memory region. Bank 0 gets the _header.o. 

```linkfile

MEMORY
{
  samsbanked (rx) : ORIGIN = 0xA000, LENGTH = 0x2000 /* 8k code space */
  ram        (wx) : ORIGIN = 0xC000, LENGTH = 0x4000 /* 16k upper ram */
}

/* memcpy will be called by the compiler in some implicit situations. */
/*  use the well known address of this routine in the Force Command module */
/*  it is at this address in every bank of the cartridge ROM */
memcpy = 0x605a;

SECTIONS
{
  OVERLAY : AT (0x10000)
  {
    .bank0 {
      objects/_header.o(.text)
      objects/b0_*.o(.text)
    }
    .bank1 {
      objects/b1_*.o(.text)
    }
  } >samsbanked

  . = 0xC000;

  .data : {
    * (.data);
  } >ram

  .bss  ALIGN(2) : {
    * (.bss);
  } >ram
}
```

## Building

The Makefile has all the same concerns as the `HELLO` example's Makefile. But the following excerpts are different...

Building the binary must be done from multiple pages. So instead of extracting the .bin directly from the .elf in one shot, we extract each page and pad each page to 8k bytes using dd. Then the pages are concatenated together in order, followed by the .data segment, which was rounded up to 4k increments. 

```Makefile
BIN_ORDER=objects/bank0.page objects/bank1.page objects/data.bin

$(FNAME).bin: $(BIN_ORDER)
	cat $(BIN_ORDER) >$@

objects/data.bin: $(FNAME).elf
	$(OBJCOPY) -O binary -j .data $< objects/data.bin
	# grow the page to a round 4k multiple
	DATASIZE=`stat --printf="%s" objects/data.bin`; \
	DATASIZE=`expr $$DATASIZE / 4096`; \
	DATASIZE=`expr 1 + $$DATASIZE`; \
	DATASIZE=`expr 4096 \* $$DATASIZE`; \
	dd if=/dev/null of=$@ bs=$$DATASIZE seek=1

objects/%.page: $(FNAME).elf
	$(OBJCOPY) -O binary -j .$(shell basename $@ .page) $< $@
	# grow the page to a round 8k
	@dd if=/dev/null of=$@ bs=8192 seek=1
```

And here the rule for compiling a .c module includes a definition of `SAMS_CURRENT_BANK` derived from the filename.

```Makefile
define bankdefs
        -DSAMS_CURRENT_BANK=$(shell echo $(basename $1) | cut -c2)
endef

objects/%.o: %.c
	mkdir -p objects; cd objects; $(CC) -c $(abspath $<) $(CFLAGS) $(call bankdefs,$<) -o $(notdir $@)

```


