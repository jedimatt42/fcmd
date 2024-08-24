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
* 40 column mode is used by default if F18A is not detected.
* FinalGROM cartridge switching (limited)
* EA5 load command
* 'call' to run scripts.
* AUTOCMD script on first drive is run on startup.
* ANSI display
* TIPIMAP to control DSKx, URIx, and AUTO mapping

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
The FC directory goes on your TIPI as TIPI.FC. It contains LOAD and FCMD

- LOAD - shim for the XB command, requires FinalGROM99 and TIPI
- FCMD - EA5 utility for FinalGROM99 to reload FCMDG.bin
- FCMDXB - XB INT/VAR 254 program for FinalGROM99 to reload FCMDG.bin

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

export TIPI_HOST_NAME=tipi.local
export CLASSIC99_DSK1_DIR=~/Documents/data/retro/TI99/classic99/DSK1/
export CART_DEPLOY_DIR=~/Documents/data/retro/TI99/
./deploy.sh

```

