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

* Directories ( TIPI, other devices are un-tested )
* F18A 80 column with an ANSI font, and fast scrolling.
* 40 column mode is used by default if F18A is not detected.
* FinalGROM cartridge switching (limited)
* EA5 load command
* 'call' to run scripts. 
* AUTOCMD script on first drive is run on startup.
* ANSI display
* TIPIMAP to control DSKx, URIx, and AUTO mapping

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
