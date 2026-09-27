# ForceCommand API Calling Convention

This document describes how a C program calls a ForceCommand API function, how
the call reaches the implementation in a banked cartridge/ROM bank, and how
banked and SAMS-banked calls differ. It reflects the current ABI as generated
into `example/gcc/fcsdk/fc_api.h`.

## Overview

A ForceCommand executable runs from the upper 24K memory expansion
(`>A000`-`>FFFF`). ForceCommand itself lives in a banked ROM (cartridge or
console replacement). API functions live in different ROM banks, so every API
call is an inter-bank call.

The client side is a `static inline` trampoline generated from the
`DECL_FC_API_CALL` macro. It does not perform the bank switch itself; it asks a
fixed routine in ForceCommand to do it.

## Fixed low-memory words

The cartridge and the executable communicate through a few words at fixed
addresses in the lower memory expansion (the same words in every build, so both
sides agree without linking):

| Address | Name | Meaning |
| --- | --- | --- |
| `>2000` | `FC_SYS` | address of `fc_api()`, the API entry point |
| `>2002` | `FC_API_INDEX` / `trampdata` | API index for the current call; also the trampoline data pointer |
| `>2004` | `FC_SAMS_TRAMP` / `api_stramp` | address of `stramp()`, the SAMS bank trampoline |
| `>2006` | `procInfoPtr` | internal: current process/SAMS page info |

These are placed by `src/api_data.c` at the start of `.data`, which the linker
places at `>2000`. See the mapfile entry for `api_data.o`.

> The SAMS data word (`>2002`) and the API index intentionally share one
> address. They are never live at the same instant: the SAMS macro writes it
> immediately before calling `stramp`; `fc_api()` reads it and immediately
> overwrites it with its own stack data pointer.

## External API call sequence

The client macro (in `fc_api_template`, generated into `fc_api.h`):

```c
#define FC_SYS       *(int *)0x2000
#define FC_API_INDEX (*(volatile int *)0x2002)

#define DECL_FC_API_CALL(index, func, return_type, arg_sig, args)     \
    static inline return_type func arg_sig                            \
    {                                                                 \
        FC_API_INDEX = (index);                                       \
        return_type(*tramp) arg_sig = (return_type(*) arg_sig)FC_SYS; \
        return tramp args;                                            \
    }
```

A call therefore proceeds:

1. The client stores the API index in `>2002` (`FC_API_INDEX`).
2. The client loads the address in `>2000` (`FC_SYS`) and calls it, passing the
   function arguments per the C ABI.
3. `fc_api()` (`src/fc_api.c`, bank 0) runs:
   - makes room on the stack for a trampoline data structure,
   - reads the API index from `@trampdata` (`>2002`),
   - stores the stack address back into `@trampdata`,
   - indexes the API table (each entry is two words: function address, bank
     address) at `BASE_ADDR + 0x80 + index * 4`,
   - loads the cartridge trampoline (`@trampoline`) with that data,
   - returns the target function's return value.
4. The cartridge trampoline (`src/trampoline.asm`) switches to the target bank,
   calls the function, switches back to the return bank, and returns to
   `fc_api()`.

`fc_api()` clobbers `r0`, uses `r10`/`r11` as the stack/return, and uses `r12`
as an index register. Argument registers `r1`-`r7` are preserved through the
trampoline.

## Bank trampoline and trampdata

`src/trampoline.asm` consumes a 3-word `trampdata` structure:

| Offset | Field | Meaning |
| --- | --- | --- |
| `>0000` | `TAR_BANK` | target bank switch address |
| `>0002` | `RET_BANK` | bank to restore after the call |
| `>0004` | `TAR_ADDR` | target function address |

For cartridge banking the bank number is latched by writing to an address in
`>6000`..`>7FFE`, so `TAR_BANK`/`RET_BANK` are addresses, not numbers.

Two consequences:

- **The external trampoline always returns to bank 0.** `fc_api()` hardcodes
  `RET_BANK = BASE_ADDR` because an executable runs with bank 0 mapped. Internal
  callers (see below) use their own bank instead.
- Because of that, an **external callback invoked from a banked routine must not
  call any API function**. The callback would return to bank 0 and then return
  into the banked caller's code with the wrong bank mapped, crashing. Keep such
  callbacks as leaf functions. (`example/gcc/dsrcat` is the worked example.)

## The API index and why it is not in `r0`

Earlier revisions passed the index in `r0`. The TI C backend uses `r0` as a
scratch register for condition/sign tests while setting up a call. For calls
with **8 or more arguments** the argument registers `r1`-`r7` are full and the
backend reuses `r0` in the setup, clobbering the index. `gfx_circle` (8
arguments) and `gfx_copy` (9 arguments) then dispatched on a wrong index and
reset the machine.

The index now travels in the fixed word `>2002`, which the backend never
clobbers. This changed the client ABI: the ROM and every executable must be
rebuilt together. If you mix a new ROM with an old executable (or vice versa),
API calls will dispatch incorrectly.

## Argument passing

ForceCommand API functions follow the tms9900-gcc C ABI:

- The first seven `int`-sized arguments are passed in `r1`-`r7`.
- Additional arguments are placed on the stack at `r10` and above, with the
  return address after them.
- `r0` is a scratch register and is not used for arguments or return values.
- Return values are returned in `r1`.
- `r10` is the stack pointer, `r11` the return address, and `r12` is used by the
  bank trampolines (saved and restored around the target call).

Two practical rules:

- Use `int` (16-bit) for parameters. Per the ForceCommand headers, **8-bit
  parameters do not pass or return correctly through the banked call ABI**.
- No API function should be declared with 8 or more arguments without care; the
  index fix makes them work, but they are still the most fragile case.

## Calling between banks inside ForceCommand

Code inside ForceCommand does not use `fc_api()`. It uses `DECLARE_BANKED` /
`DECLARE_BANKED_VOID` from `src/banking.h`, for example:

```c
DECLARE_BANKED(str_cmp, BANK(1), int, bk_strcmp, (const char* a, const char* b), (a, b))
```

The caller must `#define MYBANK BANK(n)` for its own bank. The wrapper stores a
static 3-word `trampdata` (target bank, `MYBANK` as the return bank, target
address) and calls the same `trampoline`. Unlike the external entry, the return
bank is the caller's real bank, so nested and cross-bank calls compose
correctly.

## SAMS-banked calls

ForceCommand executables can be linked into SAMS pages and call across 8K banks
with `FC_SAMS_BANKED` / `FC_SAMS_VOIDBANKED`. The build passes
`-DSAMS_CURRENT_BANK=n` per module so each caller knows its own bank.

The macros write a 3-word structure to `FC_SAMS_TRAMP_DATA` (`>2002`):

| Word | Meaning |
| --- | --- |
| 0 | target SAMS bank id |
| 1 | caller SAMS bank id (`SAMS_CURRENT_BANK`) |
| 2 | target function address |

They then call `FC_SAMS_TRAMP` (`>2004`), which is `stramp()`
(`src/sams_tramp.asm`). `stramp` maps the target pages into `>A000`/`>B000`
(relative to `procInfoPtr->base_page`), calls the function, and maps the caller
pages back. SAMS page ids are relative to the process, not absolute.

## Non-local exit

`fc_exit(status)` (`src/fc_exit.asm`) does not return through this convention.
It restores the saved `runExecutable()` stack and branches back to ForceCommand
with the status. `exit(x)` in `fc_api.h` is an alias for `fc_exit(x)`.

## Gotchas checklist

- Rebuild the ROM and all executables together after any ABI change.
- Do not call API functions from a callback that ForceCommand invokes out of a
  banked routine (see the `trampdata` return-bank note).
- Keep API parameters `int`-sized.
- Terminal output (`term_*`) and the console keyboard scan (`term_kscan`) are
  not safe while a graphics mode is active.

## Related files

- `fc_api_template` - source of the SDK header and the client macro
- `scripts/makeapi.py` - generates `fc_api.h` and the ROM API table
- `fc_api.lst` - ordered list of API functions
- `src/fc_api.c` - external API entry point (`fc_api`)
- `src/trampoline.asm` - cartridge bank trampoline
- `src/sams_tramp.asm` - SAMS bank trampoline (`stramp`)
- `src/banking.h` - `DECLARE_BANKED` / `MYBANK`
- `src/api_data.c` - fixed low-memory words and API table
- `src/fc_exit.asm` - non-local executable exit
