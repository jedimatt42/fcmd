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

# Command extensions

Command extensions are program images designed to co-operate with the environment setup by Force Command. Force Command API will be available to these extensions so they can use the same terminal input/output and leverage the disk io and other features built into Force Command.

They should be AORG 0xA000 binaries, and can use the upper 24K memory expansion fully.

The 6 byte program header is different:

Force Command may itself use SAMS, so extensions must cooperate by allocating pages through the API. Both load types, `Simple Image` and `SAMS Image` will allocate loaded pages into SAMS. If SAMS is not present, or not enough pages remain, then `SAMS Image` programs will fail to load.

## Simple Image Load Type

The first word flags the file as a Force Command program. It must have the value: 0xFCFC
The second word in the file should be 0x0000. Upto 24K of the image, following the header, is loaded into 0xA000 - 0xFFFF. The image should be a single file.
The third word is the return flag. If the value is 0xFCFC then Force Command will *not* reset the screen when the program returns.
The fourth word is the start address. Force Command will branch and link `BL` to this address. Register usage will be as follows:

| register | purpose                                     |
| -------- | ------------------------------------------- |
| R1       | address of command arguments, & return code |
| R10      | gcc stack                                   |
| R11      | return address to Force Command             |
| WP       | 0x8300                                      |

Extensions may use any of workspace 0x8300. Disk IO routines internal for ForceCommand will also use scratchpad >8320 - >832F for additional info pointer in Level 2 IO routines. 0x83A0 to 0x83AA are also used by DSR calls. GPLWS 0x83E0 - 0x83FF may also be used by some Force Command routines.

Upon return to Force Command, the workspace contents will be restored. An extension should only have to work about returning to R11. R10 will be set to
0xFFFC before entry.

## SAMS Image Load Type

We provide a loader type that automatically handles loading programs greater than 24K into additional SAMS pages.

The program will have a logical set of pages allocated to it when loaded. The Force Command API provides a paging abstraction to use, so that if Force Command consumes pages for other purposes, your program does not have to account for that. When your program is loaded, the first 6 logical pages will be mapped into the upper 24K expansion in sequence.

The first word flags the file as a Force Command program. It must have the value: 0xFCFC
The second word should be set to the number of sequential SAMS 4K pages to load.
The third word is the return flag. If the value is 0xFCFC then Force Command will *not* reset the screen when the program returns.
The fourth word is the start address. Force Command will branch and link `BL` to this address. Register usage is the same for Simple Image Load Type. This address must be within the first 6 SAMS pages, as mapped into the upper 24K, so between 0xA000 and 0xFFFF.

## PATH

Commands are searched for binary files PROGRAM images in disks and directories denoted by the PATH environment variable.

For example, a path defined as `DSK5`, `TIPI.FC`, and `IDE1.BIN` joined with semicolons `;` would look like:

```bash
PATH=DSK5;TIPI.FC;IDE1.BIN;
```

Programs are searched for left to right through the directories in the PATH.

## API

The indirection handle for the API will be placed in lower expansion memory at 0x2000. You can call Force Command API functions by placing the arguments sequentially in registers from R1 up to however many arguments are required. R10 should be set to a stack pointer that can be used by the functions. When called, an extension inherits a valid value for R10. Set R0 to the index of the API to call, and `BL @>2000`.

See examples folder for API guidance by language.

For GCC, there are inline function stubs that can be included and then called. For TMS9900 assembly, there is an equates file that can be included with COPY, and examples of how to make calls.

( Gotta start somewhere )

# VDP Memory Map

In 80 column mode, Force Command utilizes extended color attributes, and some fast scroll code for the F18A.

| Address | Size in bytes | Description         |
| ------- | ------------- | ------------------- |
| 0x0000  | 0x0960        | Image table         |
| 0x0960  | 0x00A0        | /free space-------/ |
| 0x0A00  | 0x0100        | Sprite table        |
| 0x0B00  | 0x0500        | /free space-------/ |
| 0x1000  | 0x0800        | Pattern table       |
| 0x1800  | 0x0960        | Color attributes    |
| 0x2160  | varies        | /free stack ------/ |
| 0x3FF8  | 0x0008        | Nanopeb stats       |
| 0x4000  | 0x0034        | Fast scroll routine |

Depending on your disk devices, there may be VDP stack allocations from 0x4000 downward.

In 40 Column mode:

| Address | Size in bytes | Description         |
| ------- | ------------- | ------------------- |
| 0x0000  | 0x03C0        | Image table         |
| 0x03C0  | 0x0440        | /free space-------/ |
| 0x0800  | 0x0800        | Pattern table       |
| 0x1000  | varies        | /free stack ------/ |
| 0x3FF8  | 0x0008        | Nanopeb stats       |
| 0x4000  | 0x0034        | Fast scroll routine |

IO Buffers

* lvl2 IO buffer and IO buffer merge -> IO buffer
* IO Buffer holds 17 sectors or 0x1100 bytes.
* locations are common for 40 and 80 column modes.

| Address | Size in bytes | Description         |
| ------- | ------------- | ------------------- |
| 0x2160  | 0x010A        | PAB                 |
| 0x2300  | 0x0100        | IO name buffer      |
| 0x2400  | 0x1100        | IO Buffer           |
| 0x336A  | varies        | /free stack ------/ |
