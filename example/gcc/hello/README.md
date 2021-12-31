# HELLO example

This example creates a Force Command friendly executable that prints hello to the Force Command terminal display.

Files:

- _header.asm - Force Command binary header
- hello.c - application source
- linkfile - linkfile for the gcc toolchain
- Makefile - automation for compiling, linking, and packing into a binary

## Calling Force Command Functions

```c
#include <fc_api.h>

int fc_main(char* args) {
  fc_tputs("Hello\n");
  return 0;
}
```

Force Command can pass arguments to executables in the form of a char* pointer. Typically this is the remaining
text in the command line. So the entrypoint signature is different than standard C int main(int argc, char** argv), as  Force Command does not preparse arguments for you. The entrypoint signature is instead: 

```c
int fc_main(char* args);
```

This is declared in the provided `fc_api.h`, along with all the Force Command API functions.

To print 'Hello' and a newline to the screen, we define our entrypoint function, fc_main, and in it call to the Force Command terminal-put-string function: fc_tputs. This sends the string of bytes to the terminal display to be interpretted by the ANSI terminal in Force Command.

We then return 0 to indicate program success. A non-zero value will cause Force Command to display an error message upon return.

## Binary Header

```asm
    REF fc_main
    data 0xFCFC   ; Flag for valid Force Command binary
    data 0x0000   ; SAMS page count - 0x0000 means no sams required
    data 0xFCFC   ; Flag that program promises to use screen safely
    data fc_main     ; program start address
```

This header module must be in address 0xA000 when the executable is loaded. We'll see how that is handled in the linkfile later. 

Written in 9900 asm, we first reference our entry point function ( this can have any name actually ). 

The header consists of 4 words. 

- Executable Flag - without 0xFCFC, Force Command will assume this file is not an executable.
- SAMS Page Count - Since this example does not require SAMS, we'll set this to 0. 
- Terminal Flag   - If the program leaves VDP registers and memory in a safe state, then a value of 0xFCFC indates the screen does not need to be reconfigured upon executable return to Force Command. If something other than 0xFCFC is here, then Force Command will clear the screen, and reset the display mode to the default it would auto-detect on startup.
- Entry Point Address - the address of our fc_main function.

## Linking

```linkfile

MEMORY
{
  upper_exp  (wx) : ORIGIN = 0xA000, LENGTH = 0x6000 /* 24k upper ram */
}

/* memcpy will be called by the compiler in some implicit situations. */
/*  use the well known address of this routine in the Force Command module */
/*  it is at this address in every bank of the cartridge ROM */
memcpy = 0x605a;

SECTIONS
{
  /* file header */
  header : {
    objects/_header.o(.text)
  } >upper_exp

  /* common */
  .text : {              /* all modules listed here will end up at front of each ROM bank */
  } >upper_exp

  .data : {
    * (.data);
  } >upper_exp

  .bss  ALIGN(2) : {
    *(.bss);
  } >upper_exp

}
```

This is a fairly straight forward linkfile. We only need to define 1 memory region, which is the upper 24k memory expansion of the standard TI-99/4A 32k expansion card. The lower 8k expansion is already in use by Force Command module and the gcc stack. 

All segments of the binary are linked into `upper_exp`, however we make sure we pack the .o for _header.asm first. This puts it right into address 0xA000. 

Then, the rest of our .text (code) segment, followed by staticly initialized data, and block storage segments.


## Building

The makefile is a bit larger, so we'll go through it in small chunks:

Define binaries we need to use for the build. the tms9900 gcc toolchain and xdt99 suite.

FCSDK should be modified to point to the directory you have the fcsdk directory containing the fc_api.h and other c header files.

```Makefile
GAS=tms9900-as
LD=tms9900-ld
CC=tms9900-gcc
OBJCOPY=tms9900-objcopy
OBJDUMP=tms9900-objdump
XDM99?=$(shell which xdm99.py)
FCSDK=$(abspath ../fcsdk)
```

Next, FNAME defines our final executable file name. This will be a TIFILES FIAD file. 

```Makefile
FNAME=HELLO
```

You shouldn't have to change any of the rest of this makefile as it is abstracted to find all of your source files
in this directory.

LDFLAGS sets us up to use the linkfile instead of a ton of command line argument to ld.

CFLAGS sets our compiler options. It is very valuable to use the --save-temp. This will create .s and .i files next to the .o files in the objects/ directory. The .s is the generated assembly from the compiler, and can greatly help when debugging your programs.

This section sets compile options and finds the set of .c and .asm modules to compile and link.

```Makefile
LDFLAGS=\
  --script=linkfile

CFLAGS=\
  -std=gnu99 -O2 -Werror --save-temp -I$(FCSDK)

C_SRCS:=$(wildcard *.c)
ASM_SRCS:=$(wildcard *.asm) 

OBJECT_LIST:=$(sort $(C_SRCS:.c=.o) $(ASM_SRCS:.asm=.o))

LINK_OBJECTS:=$(addprefix objects/,$(OBJECT_LIST))
```

The remainder are the makefile rules, so we by default build our final file. The gcc linker produces an .elf file. We extract that into a binary and then use xdt99 suite's xdm99.py to create a FIAD. This requires python3 as well.

```Makefile
all: $(FNAME)

$(FNAME).elf: $(LINK_OBJECTS)
	$(LD) $(LINK_OBJECTS) $(LDFLAGS) -o $(FNAME).elf -Map=mapfile

$(FNAME).bin: $(FNAME).elf
	$(OBJCOPY) -O binary $< $@

$(FNAME): $(FNAME).bin
	python3 $(XDM99) -T $< -t -f PROGRAM -n $(FNAME) -o $(FNAME)

.phony clean:
	rm -fr objects
	rm -f *.elf *.bin mapfile
	rm -f $(FNAME)

objects/%.o: %.asm
	mkdir -p objects; cd objects; $(GAS) $(abspath $<) -o $(notdir $@)

objects/%.o: %.c
	mkdir -p objects; cd objects; $(CC) -c $(abspath $<) $(CFLAGS) -o $(notdir $@)

```

## The result

- HELLO - a TIFILES FIAD loadable by ForceCommand
- mapfile - an artifact from the linker that gives the address of all our symbols, very useful for debugging
- objects - a directory with all the .o .s and .i intermediate files produced be gcc.
- HELLO.elf - the intermediate linked gcc .elf file
- HELLO.bin - our raw Force Command PROGRAM image file without a TIFILES FIAD header.

