# TIPICMD usage

Produces a tipicmdG.bin and tipicmdC.bin to be loaded by FinalGROM99.

The menu entry for the GROM is 'AUTOCMD' - choose this to load the auto-running
cartridge image, or select 'TIPICMD' if you don't want the auto-running feature.

The tool provides a command line style console interface to the TI-99/4A,
borrowing concepts from MS-DOS 2.11/3.3 era... It will also be influenced by
MSX dos, as I am a fan.

Supports:

* Directories ( TIPI, other devices are un-tested )
* F18A 80 column with an ANSI font, and fast scrolling.
* 40 column mode is used by default if F18A is not detected.
* FinalGROM cartridge switching

Unrealized Goals:

* EA5 loading
* scripting
* interactive scripting / menu generation
* TIPICFG functions
  * DSKx and URIx mapping
  * auto mapping control
  * PI halting.

There are a number of commands for file management still not hooked up, but
I have the code... just need to re-attach it.

