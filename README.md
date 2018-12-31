# gcc pattern for bank switched cartridge ROM

## Theory:

Provides a macro that can be used: BANK_CALL that will set the target function and rom-bank. It will also set the caller's rom-bank. It then casts a trampoline function written in assembly, to the signature of the function you want to call. 

You then pass your arguments by calling that cast function pointer returned by the macro.

The trampoline stores the return address. It then switches banks, and BL to the original target function. That returns to the trampoline, which subsequently restores the bank, restores the original return address, and returns.

See: banking.h for calling macro.
     main.c for example usage.

## TODO: 

* Return value gets messed up if -O0 is used... works fine with -O2 ?? Otherwise, parameters and return values work.
* Implement a stack - in this example, you cannot nest calls.
* Figure out how to link in library modules.

