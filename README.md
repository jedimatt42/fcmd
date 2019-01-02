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


