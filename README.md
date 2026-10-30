# Force Command usage

Produces a FCMDG.bin and FCMDC.bin to be loaded by FinalGROM99.

The menu entry for the GROM is 'AUTOCMD' - choose this to load the auto-running
cartridge image, or select 'FORCE CMD' if you don't want the auto-running feature.

The autorunning feature of 'AUTOCMD' skips the TI-99/4A title screen and begins
Force Command immediately.

The tool provides a command line style console interface to the TI-99/4A,
borrowing concepts from MS-DOS 2.11/3.3 era... It will also be influenced by
MSX dos, as I am a fan, and whatever seems easiest to implement...

Supports:

* Directories
  * Tested against TIPI, SCSI 1.6, IDE (Fred Kaal DSR), Myarc HFDC
* F18A 80 column with an ANSI font, and fast scrolling.
* `f18a` command to switch a V9938/F18A hybrid to F18A mode from a script.
* 80 column on V9938/V9958/F18A; 40 column on TMS9918A.
* FinalGROM cartridge switching (limited)
* EA5 load command
* 'call' to run scripts.
* AUTOCMD script on first drive is run on startup.
* ANSI display
* Output redirection with `>` (replace) and `>>` (append) to DISPLAY/VARIABLE 80 files
* `|` pipelines with temporary DV80 handoff files
* TIPIMAP to control DSKx, URIx, and AUTO mapping

### Redirection and pipelines

`>` and `>>` redirect terminal output to a DISPLAY/VARIABLE 80 file. Records
break at 80 characters, line feed, carriage return, or CRLF.

```text
dir >DSK1.DIR
echo hello >>DSK1.LOG
```

Pipelines run each stage to completion. A non-final stage's output is written to
a temporary DV80 file; the next stage reads it through `$PIPEIN`. Set `FCTMP` to
a writable directory path prefix (the current path is used by default):

```text
FCTMP=TIPI.TMP.
dir | type $PIPEIN
dir | type $PIPEIN >DSK1.LIST
```

Temporary files are named `FCPIPE0` and `FCPIPE1` and are removed when consumed
or when the pipeline ends. Stages alternate between the two slots (stage 0 uses
`FCPIPE0`, stage 1 uses `FCPIPE1`, stage 2 uses `FCPIPE0` again); the previous
slot has already been consumed and deleted before it is reused. Only the final
stage may use `>` or `>>`. Use `\|` for a literal pipe. Nested pipelines are
rejected, and pipelines cannot run inside an already-active redirection.
`STATUS` holds the first nonzero stage status. The resolved temp path must fit
in a variable value (81 characters).

Some older built-ins (notably `copy` and scripts) still return 0 after printing
an error, so pipeline status is most meaningful with external programs and the
commands that report failures.

Only output written through the ForceCommand terminal layer is captured. Direct
screen writes, TUI, graphics, and raw VDP output are outside redirection.

### Archives

`FCARC` creates archives and `FCUNARC` reads them. They are external utilities
installed as `FC/BIN/FCARC` and `FC/BIN/FCUNARC`:

```text
FCARC c /r MYARCH TIPI.PROJECT     create an archive with directories
FCUNARC l MYARCH                   list
FCUNARC x MYARCH                   extract into the current directory
FCUNARC t MYARCH                   verify
FCARC c /ti OLDARC TIPI.FILE1 ...  create a legacy TI Archiver archive
```

`FCUNARC t` prints an `OK` or `ERROR` line for each file and returns the
number of bad entries as its exit code. Extracted files are written as
TIFILES FIAD files.

`FCUNARC` reads both the new FCARC format and the legacy TI Archiver ARC/ARK
format. The legacy format is a flat archive compressed with the LZW variant
documented by Ninerpedia and implemented by TIImageTool. `FCARC c /ti` writes
legacy-compatible archives; `/ti` rejects directory sources because the
legacy format has no path names.

A dependency-free Python implementation is included as `scripts/fcarc.py`
for creating, listing, extracting, and testing both formats on a modern
computer. See `example/gcc/fcarc/README.md` for the format specification and
usage details.

## End Users Wiki

The following wiki contains documentation for how to use, and where to find the pre-built binaries:

[Force Command Github Wiki](https://github.com/jedimatt42/fcmd/wiki)

I also post the binaries on Atariage:

[Atariage Thread](https://atariage.com/forums/topic/290966-force-command-ver-132-kinda-like-commandcom-from-1985-no-tipi-required/)

### Help

There is a built in help command, it will list the commands/topics, then `help <topic>` for further help. These are also all documented on the [Force Command Github Wiki](https://github.com/jedimatt42/fcmd/wiki)

## Building

Prerequisites:

* xdt99 suite - [xdt99 github repository](https://github.com/endlos99/xdt99)
* python3.8+
* tms9900-gcc-1.31 - [tms9900-gcc github repository](https://github.com/mburkley/tms9900-gcc)

```bash
make clean
make
```

This will compile all the binaries.

The host-side archive tool tests can be run with:

```bash
make test-fcarc
```

### Visual Studio Code

The clangd extension works well, but you need to use `bear` to generate info on how it compiles, what include dirs are 
used, etc. 

To produce the compile_commands.json:

```
make clean
bear -- make
```

## Installation on real hardware

The FCMDG.bin and FCMDC.bin go on a FinalGROM99 - or if you know what you are doing, an UberGROM

The FC directory goes on your TIPI as TIPI.FC, preserving its subdirectories:

- FC/LOAD - shim for the XB command, requires FinalGROM99 and TIPI
- FC/FCMD - EA5 utility for FinalGROM99 to reload FCMDG.bin
- FC/FCMDXB - XB INT/VAR 254 program for FinalGROM99 to reload FCMDG.bin
- FC/BOOT - EA5 program for console ROM replacement builds to switch back to FCMD
- FC/BIN - command utilities and fonts: CHARSET, DISKIMAGE, FCARC, FCUNARC, FCMDANSI, FCMENU, FONT, FTP, MDOSANSI, MDOSFONT, MSXFONT, SAMPLE, SAY, TELNET, VIRGIL99

Copy the FC directory, including the FC/BIN subdirectory, to TIPI.FC without flattening it:
the utilities are expected in TIPI.FC.BIN. `deploy.sh` does this when `TIPI_DIR` points at
the TIPI disk (for example `/media/tipi/tipi_disk`).

### Emulators

* Classic99 ini entries:

Add a uniquely numbered `[UserCart` nnn `]` section.

```ini
[UserCart0]
name=Force Command
rom0=8|00000|20000|<path-to-your-files>/FCMDC.bin
rom1=G|6000|2000|<path-to-your-files>/FCMDG.bin
```

* MAME & Js99er.net

FCMD.RPK : contains both the ROM and auto start GROM of Force Command.

### deploy.sh script

There is a script named `deploy.sh` that I use to copy the build artifacts to my TIPI, and classic99 emulation environment, etc... If some variables are not set, that destination will be skipped. To use, I wrap in a custom script that sets control variables:

```bash
#!/bin/bash

make clean
make -j 12

# Not actually necessary, used to update example/gcc/say/sample.spc from sample.wav
export PYWIZ_DIR=$HOME/github/python_wizard
make -C example/gcc/say sample.spc

export TIPI_DIR=/media/tipi/tipi_disk
export CLASSIC99_DSK1_DIR=~/Documents/data/retro/TI99/classic99/DSK1/
export CART_DEPLOY_DIR=~/Documents/data/retro/TI99/
./deploy.sh

```

