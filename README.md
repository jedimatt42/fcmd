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

## Installation

The FCMDG.bin and FCMDC.bin go on a FinalGROM99 - or if you know what you are doing, an UberGROM
The FC directory goes on your TIPI as TIPI.FC. It contains LOAD and FCMD

LOAD - shim for the XB command, requires FinalGROM99 and TIPI
FCMD - EA5 utility for FinalGROM99 to reload FCMDG.bin
FCMDXB - XB INT/VAR 254 program for FinalGROM99 to reload FCMDG.bin

## Help

There is a help command, it will list the commands/topics, then `help <topic>` for further help.

## Example

This example of a MENU script can be saved to a D/V80 file and run in Force Command.
It shows a number of things:

1. setting color
1. defining variables
1. using ANSI escape codes in 'echo' statements
1. reading a keypress into a variable
1. using variables and if statements
1. switching cartridge images
1. goto statements
1. loading EA5 programs
1. setting DSK1 mapping on TIPI
1. looping

```
color 1 7
cls
CLA=2
CLB=25
CLC=50
echo \e[2;20HJedimatt's TI-99/4A
echo \e[4;$(CLA)HX - Extended Basic
echo \e[6;$(CLA)HF - fbForth
echo \e[8;$(CLA)HT - Telnet
echo \e[4;$(CLB)HP - Parsec
echo \e[6;$(CLB)HD - Tunnels of Doom
echo \e[8;$(CLB)HC - Old Dark Caves
echo \e[4;$(CLC)HA - TI Artist Plus
echo \e[20;$(CLC)HQ - Quit
INPUT:
echo /n \e[24;1H
readkey K
if $(K) == X then fg99 TIXB_G
if $(K) == F then goto LFB
if $(K) == T then load TIPI.NET.TELNET
if $(K) == P then load TIPI.GAMES.EA5.PARSEC1
if $(K) == D then goto TOD
if $(K) == A then goto ARTIST
if $(K) == C then goto CAVES
if $(K) == Q then goto END
goto INPUT

LFB:
tipimap DSK1 TIPI.FB4TH210
fg99 FBFORTH

TOD:
tipimap DSK1 TIPI.GAMES.TOD
load TIPI.GAMES.TODM.TOD:1

ARTIST:
tipimap DSK1 TIPI.DISKS.INSCEBOT
fg99 TIXB_G

CAVES:
tipimap DSK1 TIPI.DISKS.CAVES
fg99 TIXB_G

END:
color 15 4
cls
ver

```

# External Commands (intent, not yet released)

Force Command allows loading external commands that can receive command line arguments and utilize an API provided by the Force Command ROM.

External commands are loaded into the top 16k of memory expansion 0xC000 - 0xFFFF.

## API (intent, not yet released)

Force Command has a table of ROM routines in BANK(0), starting at address 0x6070. Each entry is the function address and the bank switch write address. The api covers the following categories of routines:

> tputs, getstr, level2 disk routines, level 3 disk routines, tipi messaging and tcp routines

Calling and documentation for these routines are available in the github wiki.

Force Command searches for external commands only after not detecting a built-in command. It looks in the list of folders defined in the environment variable PATH. For instance, to set Force Command to search folders TIPI.BIN and TIPI.CONTRIB.BIN:

> PATH=TIPI.BIN;TIPI.CONTRIB.BIN

External commands are PROGRAM image files but, instead of the EA5 header, they have a Force Command header:

| byte offset | value  | description |
| ----------- | -----  | ------------|
| 0, 1        | 0xFCFC | Tag indicating a Force Command binary |
| 2, 3        | start  | Start address of program entry point |

Files should be single file PROGRAM image format. Only upto the first 16k of the file will be loaded. Regardless of the start address, the image will be loaded into address 0xC000 (without the 6 byte header). Force Command will call your start address as though it had the C function signature:

> void main(int argc, const char** argv)

If the external command is written in C, it should use the same workspace and C stack. The C stack is limited to about 2k. No crt.asm/crt.c is needed, your function will be called with the workspace and your C stack set for you.

* WP = 0x8300
* C stack top of ram is 0xC000.

Force Command uses memory from 0x2000 - 0x3FFF, and 0xA000 - 0xBFFF, and scratchpad 0x8300 - 0x831F.

| Address Range | Usage |
| ------------- | ----- |
| 0x0000-0x1FFF | 8k System ROM |
| 0x2000-0x3FFF | 9k Force Command common library |
| 0x4000-0x5FFF | 8k Expansion DSR ROM |
| 0x6000-0x7FFF | 128k Force Command bank switch ROM |
| 0x8000-0x9FFF | Memory Mapped Console IO |
| 0xA000-0xBFFF | 8k Force Command RAM data and stack |
| 0xC000-0xFFFF | 16k available for external command |

Force Command uses a stack for bank switching the cartridge ROM. When you function is called, you will be first switched to BANK(0), where the api address table is present.

## fc_api.h

Force Command provides a C header with macros to make life easier. API calls
