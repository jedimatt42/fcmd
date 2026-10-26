# ForceCommand 3.0 changelog (unreleased)

Changes since **v2.6**. Features already shipped in the v2.x releases are not
repeated; that includes the v2.6 TIPIMAP `CS1`/`DSK5`–`DSK9` mappings.

## User-visible features

### Shell and scripting

- **Command aliases** with the new `alias` command.
  - `alias name value` defines an alias; the value may include arguments.
  - Additional command-line arguments are appended to the alias value.
  - `alias name` removes an alias; `alias` with no arguments lists all aliases.
- **Output redirection** with `>` (replace) and `>>` (append).
  - Writes terminal output to a DISPLAY/VARIABLE 80 file.
  - Records break at 80 columns, LF, CR, or CRLF.
  - Screen output is suppressed while redirected; `\>` produces a literal `>`.
- **Pipelines** with `|`.
  - Each stage runs to completion and hands its output to the next stage through
    a temporary DV80 file.
  - The next stage reads the file through `$PIPEIN`; the temp location can be
    set with `FCTMP`.
  - Temp files are `FCPIPE0`/`FCPIPE1`, removed when consumed or when the
    pipeline ends.
  - `\|` is a literal pipe; nested pipelines and pipelines inside an active
    redirection are rejected.
- **Command exit status** is now carried through the shell.
  - Every built-in returns a status, and the `STATUS` variable is updated after
    every command.
  - `STATUS` reflects the first non-zero status in a pipeline.
  - Some older built-ins (notably `copy` and scripts) still return 0 after
    printing an error, so pipeline status is most useful with commands that
    report failures.
  - Executables can call `exit(n)` to leave immediately and return a status;
    non-zero statuses still print `error result: n`.
  - `exec_cmd` returns the command's result code to the caller.
- **Variable expansion before assignment**: substitutions and escapes are
  processed before an assignment, so `A=$B` stores the expanded value.
- **Help topics expanded and separated**.
  - Shell topics (`autocmd`, `label`, `path`, `pipeline`, `redirect`, `f18a`)
    are listed separately from commands.
  - Added help for aliases, redirection, pipelines, and the `f18a` command.
- **`dir` enhancements**.
  - `dir /d` lists directory entries only.
  - `/w` listings mark directories with `[]`.
  - Options can be combined (`dir /wd`).
  - Long names and wide listings wrap without splitting columns.
- **New `f18a` command**.
  - Switches a V9938/F18A hybrid card to the F18A personality and starts its
    80×30 color text mode.
  - Intended for use from `AUTOCMD`; a power cycle is required to return a
    hybrid card to V9938 mode.
- **`DSK.` device support**: `DSK.` is treated as unit 0 for PICO cart-style
  devices.
- **Diagnostics**:
  - `drives /a` shows DSR routine addresses and marks devices with CPU-buffer
    support as `{c}`.
  - `lvl2 /a` shows level-2 routine addresses.
  - `sysinfo` reports the GCC patch level when the toolchain provides it.
  - `ver` identifies the running build as `8-bit CART` or `16-bit ROM`.

### Display, hardware, and I/O

- **Console ROM replacement build**.
  - New 512 KB `FCMD_0000.bin` image places ForceCommand in console ROM space
    and retains the original console ROM at the top of the image.
  - `FCMDG1.bin` is a patched GROM 1 that autoboots ForceCommand.
  - `FC/BOOT` is an EA5 utility that clears the console-ROM bank and returns to
    ForceCommand after running a cartridge.
- **Improved VDP detection**.
  - F18A probing is only attempted when no Yamaha VDP answers, avoiding an
    accidental, permanent personality switch on V9938/F18A hybrid cards.
  - The new `f18a` command performs that switch deliberately.
  - F18A detection also recovers correctly after a soft reset from 80×30 mode.
- **Improved PAL/NTSC detection** by measuring the VDP interrupt rate over ten
  frames with the CRU timer, giving a wider and more reliable threshold.
- **SAMS performance**: SAMS wait states are disabled when the hardware supports
  it.
- **CPU-buffer I/O**.
  - File, direct-input/output, and sector I/O use CPU-side PAB buffers when the
    storage card supports them.
  - Supported/detected devices include TIPI, Horizon RamDisk, Myarc HFDC, IDE,
    and SCSI controllers.
  - Non-CPU-buffer cards fall back to the VDP path.
- **Font/tool packaging**.
  - Added `FC/BIN/FCMDANSI` (the default ForceCommand ANSI font) and
    `FC/BIN/MSXFONT` (MSX font).
  - `FC/BIN/CHARSET` is now shipped.
  - MDOS fonts moved under `FC/BIN`.
  - The release archive now contains the console-ROM image, patched GROM, RPK,
    cartridge image, support files under `FC/BIN`, and the C SDK.

### Fixes to existing 2.x behavior

- Fixed the EA5 `LOAD` command.
- Fixed path parsing so `delete` without a filename can no longer act like a
  wildcard delete.
- Fixed heap allocation overwriting the environment dictionary (variables and
  aliases).
- Fixed `str_cmp`/`str_cmp_icase` to compare the first character as well.
- Removed the spurious `DEVICE NOT FOUND`/`No parent folder` messages printed
  during ordinary path resolution at boot.

## Architectural changes

### Source layout and build

- Source tree reorganized into `src/`, `src/libti99/`, `scripts/`, and `FC/`.
- Build products moved into `build_0x6000/` (cartridge) and `build_0x0000/`
  (console ROM); both are built by default.
- ROM bank assignment now comes from each source file's `#define MYBANK
  BANK(n)`. `scripts/gen-linkfile-m4.py` generates the linker script, replacing
  the old bank-name convention.
- New build and packaging scripts:
  - `scripts/gen-linkfile-m4.py` for bank layout.
  - `scripts/makeapi.py` for the API table and SDK header.
  - `scripts/patch_grom1.py` for console-ROM autoboot.
  - `scripts/ans2asm.py` for the boot banner.
  - `scripts/api_coverage.py` for API coverage.
- The release build now compiles the ROM, support tools, SDK, and all example
  and test programs, and packages everything into `forcecmd_3.0.zip`.
- Switch to `-Os` and removal of `__packed__` from structs recovered ROM space
  and removed alignment/packing differences between the ROM and executables.

### Console ROM and banking

- Added a complete console-ROM build path with:
  - a 512 KB image layout,
  - a patch to GROM 1 that jumps to the ForceCommand entry point on power-up,
  - a console-ROM variant of the bank-switch trampoline,
  - console-ROM trampolines for KSCAN and other console services while the
    console ROM is banked out,
  - interrupt-safe entry/exit and EA5 launching,
  - `FC/BOOT` to return from a cartridge to the console-ROM build.
- Both cartridge and console-ROM images use the same API table layout, with the
  entry point at `>2000` and the API index/trampoline data word at `>2002`.

### Executables and API ABI

- API calls are inter-bank calls through a common trampoline replicated at the
  front of every ROM bank.
- The API index moved from `r0` to the fixed word `>2002`, removing a source of
  dispatch corruption when the compiler used `r0` while setting up a call.
- Banked-callable functions are limited to seven `int` arguments. APIs that need
  more parameters now take a struct pointer (for example `gfx_copy`), and
  variadic APIs were removed.
- External executable callbacks passed to `dsr_catalog` and
  `term_set_identify_hook` now run through the common trampoline with bank 0
  mapped, so they may call ForceCommand APIs. ROM callbacks remain direct calls.
- Program exit is no longer limited to returning from `main`: `fc_exit`/`exit`
  restores the saved `runExecutable` stack and returns the requested status.
- API functions, the SDK header, and the API table are generated together, so
  the ROM and all executables must be rebuilt as a unit.

### Runtime and compiler support

- Added libgcc-compatible double/float runtime support in ROM bank 0:
  add, subtract, multiply, divide, comparisons (`<`, `<=`, `>`, `>=`, `==`,
  `!=`), integer-to-double, and double-to-integer.
- Added `memcpy` and `memset` runtime routines.
- Moved `ftoa` into the ROM and added better float formatting and
  `str_from_float`.
- `fc.ld` is generated from the ROM mapfile so executables resolve runtime
  symbols through branch-table addresses instead of hard-coded cartridge
  addresses.
- Added runtime support for the ForceCommand Expanse compiler: SAMS page mapping and
  dereferencing of 32-bit SAMS heap handles, plus a SAMS-backed `malloc`/`free`.
- Public structures are no longer packed; `DateTime.dayOfWeek` moved to the end
  of the structure.
- Public API structs are shared between the ROM, SDK, and executables, so all
  sides must be recompiled together.

### I/O and display integration

- VDP filesystem buffers are now relocatable instead of fixed addresses. Each
  successful graphics mode change reserves PAB, parameter, and file buffers and
  updates DSR/level-2/system-information users to the new addresses.
- The no-SAMS VDP history buffer is only enabled in text modes and is cleared
  when entering a text mode, so graphics sessions cannot corrupt or expose it.
- Graphics modes reset the console hooks: terminal output becomes inert in
  bitmap/multicolor modes, scrolling is disabled, and `term_kscan` runs with
  interrupts disabled.
- CPU-buffer support was threaded through DSR, level-2, clock, TIPIMAP, XB, and
  other file-reading paths.
- Mouse pointer sprite patterns moved from VIRGIL99 into the ROM, so the TUI and
  other applications share the same normal/busy/scroll/page pointers.

### Tests and documentation

- Added 39 API test/demo programs covering all 204 API entries.
- Added the shared `apitest` harness, `scripts/api_coverage.py`, and the
  `APITEST.DSK` build target.
- Ported the bundled examples and tools (VIRGIL99, FCMENU, FTP, TELNET, etc.) to
  the v3 API and CPU-buffer I/O.
- Added `internal-docs/api-calling.md`, `graphics.md`, `tui.md`, and
  `soundplayer.md`, plus CaDD VDP and TI technical reference material.
- Updated the README with redirection/pipeline behavior, the `FC/BIN` layout,
  the console-ROM boot utility, emulator setup, and build/deploy instructions.

## API support

### Breaking changes

- The GCC API is **not source or binary compatible with ForceCommand 2.x**.
  - Legacy `fc_` prefixes were removed and names normalized to subsystem
    prefixes. The new `fc_exit` is the exception.
  - API table indices were reordered.
  - The calling convention now uses the fixed index word at `>2002`.
  - Public structures changed layout (packing removed, fields reordered).
  - `display_info` was removed; use `gfx_get_info` and `sys_info`.
- Examples of the rename pattern:
  - `fc_tputs` → `term_puts`, `fc_tputc` → `term_putc`, `fc_getstr` →
    `term_gets`, `fc_kscan` → `term_kscan`
  - `fc_vars_get`/`fc_vars_set` → `var_get`/`var_set`
  - `fc_exec` → `exec_cmd`
  - `fc_uint2str` → `str_from_uint`, `fc_uint2hex` → `hex_from_uint`,
    `fc_atoi` → `str_to_int`, `fc_htoi` → `hex_to_int`
  - `fc_tipi_mouse` → `mouse_read`, `fc_tipi_mouse_move` → `mouse_move`,
    `fc_tipi_mouse_enable` → `mouse_show`, `fc_tipi_mouse_disable` →
    `mouse_hide`
  - `fc_datetime` → `time_get`
  - `fc_detect_speech` → `speech_detect`, `fc_say_vocab`/`fc_say_data` →
    `speech_say_vocab`/`speech_say_data`
  - `fc_init_socket_buffer`/`fc_readline`/`fc_readstream` → `sockbuf_init`/
    `sockbuf_readline`/`sockbuf_readstream`
- Existing 2.x executables must be recompiled against the v3 SDK before they can
  run with a v3 ROM.

### New API areas

- **Graphics (`gfx_*`, 27 functions)**
  - Modes for TMS9918A Graphics 1/2/Multicolor, V9938/V9958 Graphics 3–7 and
    Text80, V9958 YJK, and F18A Text80×30.
  - Capability/error reporting (`GFX_OK`, `GFX_ERR_*`, capability bits) and
    `gfx_get_info`.
  - Pixels, lines, boxes, circles, ellipses, paint, vector strings (`gfx_draw`),
    page selection, and overlapping-safe `gfx_copy`.
  - Tiles and sprites, including partial sprite updates (`gfx_sprite_loc`,
    `gfx_sprite_color`, `gfx_sprite_pattern_index`, hide/enable/status).
  - F18A palette writes (`gfx_palette_set`).
- **Text UI (`tui_*`, 65 functions)**
  - Caller-provided memory pool; no ROM-side allocation.
  - Windows, borders, titles, scrolling, and window-relative output.
  - Widgets include buttons, checkboxes, labels, lists, menus, progress bars,
    text fields, and windows.
  - Dialogs: message box, confirm box, and input box.
  - Focus navigation, non-blocking event polling/dispatch, and mouse clicks.
- **Sound lists (`snd_*`, 5 functions)**
  - `snd_start`, `snd_tick`, `snd_playing`, `snd_stop`, and `snd_play` for
    VDP sound lists.
- **Mouse (`mouse_*`, 5 functions)**
  - The old `tipi_mouse_*` calls are now `mouse_read`, `mouse_move`,
    `mouse_show`, and `mouse_hide`; `mouse_set_pointer` is new.
  - Pointer styles for normal, busy, scroll, and page actions.
  - Button bits are masked to `MB_LEFT`/`MB_RIGHT`/`MB_MID`.
- **SAMS**
  - Added `sams_read_page` so programs can save/restore a mapped page when
    mixing direct SAMS page use with the heap.
  - The existing alloc/free/map operations are available as
    `sams_alloc_pages`, `sams_free_pages`, and `sams_map_page`.
- **Filesystem and CPU-buffer I/O**
  - Added `dsr_scratch` and `dsr_read_cpu`; `dsr_find` replaces `find_dsr`.
  - Added `lvl2_input_cpu`, `lvl2_output_cpu`, and `lvl2_format`; `path_parse`
    replaces the old path-parsing helper.
  - The DSR, level-2, and sector capabilities remain available under the new
    names.
- **Program control**
  - Added `fc_exit` (also available as `exit`) for non-local exit with status.
  - `exec_cmd` now returns the executed command's result code.
- **TUI mouse**
  - Added `tui_mouse_enable` and `tui_mouse_disable`.
  - Dialog helpers enable/disable the mouse automatically; applications using
    their own event loop can enable it explicitly.

### Smaller API changes

- `tui_win_printf` is no longer variadic. Its signature is
  `tui_win_printf(win, fmt, count, args)`.
- `hex_from_uint`, `hex_to_int`, `str_from_uint`, `str_from_basic`, and
  `str_from_float` replace and extend the old conversion helpers.
- `str_cmp`/`str_cmp_icase` now compare the first character correctly.
- The SDK header is generated into `fcsdk/fc_api.h` and is also kept in
  `example/gcc/fcsdk/fc_api.h` for the examples.
- `fc.ld` now provides the runtime symbols (`memcpy`, `memset`, double/float
  helpers) expected by the compiler.

## Upgrade notes

- Rebuild the ForceCommand ROM and **all** executables together. Mixing a v2
  executable with a v3 ROM, or the reverse, will dispatch API calls incorrectly.
- Rename API calls to the v3 names. A search-and-replace of the `fc_` prefix is
  not sufficient; use the generated `fc_api.h` as the reference.
- Convert any API call with more than seven arguments to a struct pointer; the
  same limit applies to callbacks.
- Recompile programs that use public structures; packing and some field orders
  changed.
- Rebuild programs that use `str_from_float`, `memcpy`, `memset`, or
  float/double arithmetic against the new SDK so they link through the generated
  `fc.ld`.
- Console-ROM users must deploy both `FCMD_0000.bin` and the patched
  `FCMDG1.bin`; `FC/BOOT` provides a way back from a cartridge.
- Update the TIPI `FC` directory preserving subdirectories; bundled utilities
  and fonts now live under `FC/BIN`.
