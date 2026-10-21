# Executable Examples

ForceCommand SDK programs and the API test suite.

## Building

```
make -C example/gcc/<directory>
```

Each directory produces a TIFILES executable (the uppercase program name, e.g.
`example/gcc/gfxcircle/GFXCIRCLE`). Copy it to a ForceCommand-visible device and
run it by name.

The API tests use `../apitest/fctest.h`; results are printed as
`[PASS]`/`[FAIL]`/`[SKIP]`/`[OBSERVE]` lines and a final
`pass=N fail=N skip=N` (failed checks are also re-listed under `failed:`).
`main()` returns the failure count, so a non-zero result is reported by
ForceCommand.

`make -C example/gcc/<dir>` is enough after a ROM rebuild; `make subdirs` from the
repository root rebuilds all examples.

## API test suite

| Inc | Directory / program | Covers |
| --- | --- | --- |
| I0  | [apitest](apitest) / `APITEST` | harness self-test |
| I1  | [sysinfo](sysinfo) / `SYSINFO` | `sys_info`, `sys_sams_info`, `time_get`, hex conversion |
| I2  | [strtest](strtest) / `STRTEST` | string functions |
| I3  | [strfmt](strfmt) / `STRFMT` | `str_from_int`/`str_from_uint`/`str_from_float` |
| I4  | [listtest](listtest) / `LISTTEST` | `list_*` |
| I5  | [vartest](vartest) / `VARTEST` | variable store |
| I6  | [exectest](exectest) / `EXECTEST` | aliases and `exec_cmd` |
| I7  | [termtest](termtest) / `TERMTEST` | terminal colors/output |
| I8  | [termread](termread) / `TERMREAD` | terminal input |
| I9  | [vdptool](vdptool) / `VDPTOOL` | low-level VDP helpers |
| I10 | [audio](audio) / `AUDIO` | `audio_*` |
| I11 | [sndtest](sndtest) / `SNDTEST` | sound list |
| I12 | [exittest](exittest) / `EXITTEST` | exit paths |
| I13 | [samstest](samstest) / `SAMSTEST` | SAMS memory |
| I14 | [dsrtest](dsrtest) / `DSRTEST` | DSR access |
| I15 | [dsrcat](dsrcat) / `DSRCAT` | catalog callbacks |
| I16 | [lvl2file](lvl2file) / `LVL2FILE` | level-2 file I/O |
| I17 | [lvl2dir](lvl2dir) / `LVL2DIR` | level-2 directory |
| I18 | [lvl2sector](lvl2sector) / `LVL2SECTOR` | level-2 sector I/O |
| I19 | [mousetest](mousetest) / `MOUSETEST` | `mouse_read` |
| I20 | [tipitest](tipitest) / `TIPITEST` | TIPI messaging |
| I21 | [socktest](socktest) / `SOCKTEST` | TCP sockets |
| I22 | [tlstest](tlstest) / `TLSTEST` | TLS |
| I23 | [speechtest](speechtest) / `SPEECHTEST` | speech |
| I24 | [gfxmodes](gfxmodes) / `GFXMODES` | graphics mode walk, caps, FU2 tile text |
| I25 | [gfxpixels](gfxpixels) / `GFXPIXELS` | `gfx_pset`/`preset`/`point`/cursor/ops |
| I26 | [gfxlines](gfxlines) / `GFXLINES` | `gfx_line` styles |
| I27 | [gfxcircle](gfxcircle) / `GFXCIRCLE` | `gfx_circle`/`gfx_ellipse` |
| I28 | [gfxpaint](gfxpaint) / `GFXPAINT` | `gfx_paint`, `gfx_draw` |
| I29 | [gfxpages](gfxpages) / `GFXPAGES` | `gfx_set_page`, `gfx_copy` |
| I30 | [gfxtiles](gfxtiles) / `GFXTILES` | tiles and sprites |
| I31 | [gfxpal](gfxpal) / `GFXPAL` | `gfx_palette_set` (F18A) |
| I32 | [gfxattr](gfxattr) / `GFXATTR` | 80-column attributes/tiles |
| I33 | [tuidraw](tuidraw) / `TUIDRAW` | TUI primitives |
| I34 | [tuiwin](tuiwin) / `TUIWIN` | windows |
| I35 | [tuiwidgets](tuiwidgets) / `TUIWIDGETS` | widget create/accessors |
| I36 | [tuievents](tuievents) / `TUIEVENTS` | events/focus |
| I37 | [tuidialogs](tuidialogs) / `TUIDIALOGS` | dialogs and menus |

### Test disk and coverage audit

```
make APITEST.DSK        # collect the I-series FIADs on a 360K disk
make apitest-dsk        # same target by name
python3 scripts/api_coverage.py            # per-API coverage
python3 scripts/api_coverage.py --missing  # uncovered names
python3 scripts/api_coverage.py --groups   # per-prefix summary
```

## Other demos

- [hello](hello) / [samshello](samshello) — minimal cart and SAMS programs.
- [graphicsdemo](graphicsdemo) — graphics API demo.
- [tuidemo](tuidemo) — interactive TUI demo.
- [gfxbitmap](gfxbitmap) — graphics mode-transition probe (`GFXBITMAP [flags] [modes]`).
- [charset](charset), [diskimage](diskimage), [fcmenu](fcmenu), [ftp](ftp),
  [say](say), [telnet](telnet), [virgil](virgil), [font](font) — application
  examples.
