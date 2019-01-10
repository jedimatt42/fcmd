# gcc pattern for bank switched cartridge ROM

## Theory:

Allow creating static inline bank switching calls that delegate through a trampoline and maintain a stack of bank switch
calls.

See: banking.h for calling macro.
     main.c for example usage.

## TODO: 

* Return value gets messed up if -O0 is used... works fine with -O2 ?? Otherwise, parameters and return values work.

## Stack depth

A limit is defined in the Makefile. BANK_STACK_SIZE

## Libraries:

complicated:

Library linking works... by default they will get pulled into the common .text segment... they won't all fit.
You will need to select which bank the library routines belong in, and select them in with the linkfile.
See this project's linkfile for examples using libti99.

You may have a routine you need to be commonly available in all banks. This should then go in the common .text segment,
and you'll have to shrink the overlay segment, and increase the common segment. The common segment size is defined in the
makefile, and in the linkfile.

## Files:

* banks.h - defines constants for the write addresses that trigger cartridge bank switching
* banking.h - defines the DECLARED_BANK macro and related trampoline symbols
* bankdata.c - module defining storage for the bank_stack, and trampoline parameter location
* trampoline.asm - module defining trampoline function
* Makefile - defines stack size, extracts binary chunks and composes cartrige bin
* linkfile - defines which modules end up at what address spaces.
* crt0.c - defines startup routine for C, and memory initialization, calls main.
* header.asm - defines cartridge header for TI-99/4A, sets bank_0, and calls c startup


