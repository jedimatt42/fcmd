# TIPICMD usage

Produces a tipicmdG.bin and tipicmdC.bin to be loaded by FinalGROM99.

The menu entry for the GROM is 'AUTOCMD' - choose this to load the auto-running
cartridge image, or select 'FORCE CMD' if you don't want the auto-running feature.

The tool provides a command line style console interface to the TI-99/4A,
borrowing concepts from MS-DOS 2.11/3.3 era... It will also be influenced by
MSX dos, as I am a fan, and whatever seems easiest to implement...

Supports:

* Directories ( TIPI, other devices are un-tested )
* F18A 80 column with an ANSI font, and fast scrolling.
* 40 column mode is used by default if F18A is not detected.
* FinalGROM cartridge switching (limited)
* EA5 load command
* 'call' to run scripts
* ANSI display
* TIPIMAP to control DSKx, URIx, and AUTO mapping

## Help

There is a help command, it will list the commands/topics, then help <topic> for further help.

## Todo

'if' command is defined, but does not work.

