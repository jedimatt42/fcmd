#  The TI-Writer module

This cartridge is hardly more than a dongle: the main programs, the text
editor and the text formatter, come on a companion disk. All the
cartridge does is:

Select the language to be used, which implies loading special character
definitions, and translate the prompts.

Present the user with the main menu, and launch the selected program.

List a disk directory (stangely this function is not available from the
main menu, but must be called from the editor).

[Language selection
](#program)[Initialisation
](#Init)[Main menu
](#main%20menu)[\_Editor
](#editor)[\_Formatter
](#formatter)[\_Utility
](#utility)[Directory listing](#directory)

You'll find a commented disassembly listing of the GROM in these two
files: [wr1.txt](wr1.txt) (GPL program), and [wr2.txt](wr2.txt) (data).

##  Program

The cartridge contains seven programs (and no power-up routines,
subprograms, nor DSRs). In fact, these are just seven different entry
points to the main menu, each for a different language:

`PROCESSADOR DE TEXTO` (spanish)
`TI-TEKSTWERVERKER` (dutch)
`TI-SKRIVARE` (swedish, I think)
`TRATAMENTO TESTI` (italian)
`TI-TEXTSYSTEM` (german)
`TEXTE-TI` (french, sort of)
`TI-WRITER` (english)

Due to the fact that programs are listed on the main menu screen in
reverse order of their position in the program chain, TI-WRITER will
appear first, just under TI-Basic and the spanish version is at the
bottom of the list.

Once the user selects a language, each entry point loads a different
value in byte \>8390: 1 for english, 2 for french,... 7 for spanish. It
also loads a pointer in word \>8388-8389, that points to a table of
prompts and messages in the appropriate language.

The table begins with 26 pointers. The first 25 points at 24 strings
(pointers 6 corresponds to an empty string) used to display the main
menu, the disk directory and a few messages in the selected language.
The last pointer points at the end of the table, because the display
routine calculates the length of a string by substracting its pointer
from the next one.

Just after the table is a serie of character patterns used to display
the special characters in non-english languages.

|          |        |        |                            |
|----------|--------|--------|----------------------------|
| Language | \>8390 | \>8388 | Special char pats          |
| English  | \>01   | \>69C4 | none                       |
| French   | \>02   | \>6B4B | 64, 91-93, 123-126         |
| German   | \>03   | \>6D31 | 64, 91-93, 123-126         |
| Italian  | \>04   | \>6F1D | 91, 93, 96, 123-126        |
| Swedish  | \>05   | \>70F9 | 36, 64, 91-94, 96, 123-126 |
| Dutch    | \>06   | \>72DB | 6 chars, not used          |
| Spanish  | \>07   | \>74BB | 35, 91-93, 123, 124        |

Once these pointers are set, all programs branch to a common point and
go on with VDP initialisation.

###  Initialisation

The VDP registers are then set with the following values:

R1=20 Standard mode, screen off.
R2=00 Screen image at \>0000
R3=0E Color table at \>0380
R4=01 Char pattern table at \>0800
R5=06 Sprite attribute table at \>0300
R6=00 Sprite pattern table at \>0000 (not the same as char pats!)
R7=F5 White on blue

Sprites are disabled by writing \>D0 at VDP address \>0300 (first sprite
vertical position).

Then the program loads the built-in character sets from the console
GROM, and special characters from the cartridge GROM:

The current pattern for character 10 is copied to character 255
(copyright symbol).

The small upper-case chars 32-95 are loaded at VDP address \>0900-0AFF.

The lower-case characters 96-127 at VPD address \>0B00-0BF7.

The patterns for characters 0-31 are taken from the cartridge GROM at
address \>68C4.

The special characters for each language are loaded as indicated in the
table above. English has no such characters, Dutch has six, but they are
not loaded, possibly because they are not needed to display the main
menu, nor the disk catalogue.

For some reason, provision is made to load a different set of standard
characters, possibly corresponding to another console version: the
distinction is made by scanning the keyboard with keyboard type 5. If
the type is not changed to 0 by the scanning routine, characters \>32-95
will be loaded at addresses \>0A00-0BFF in VDP memory, then characters
64-95 are modified by adding a small 2-pixel dash in their upper left
corner. I have no idea what this is meant for, the TI-99/4 may be?

Finally the screen is cleared, the color table is initialised as white
on blue (\>F5) and the main menu is displayed. VDP register 1 is changed
to \>60, which turns the screen on.

The program then checks for the presence of the memory expansion card by
writing a test value (\>BA) at \>A000 and reading it back. An error
message is displayed if the read value does not match
("`MEMORY EXPANSION NOT FOUND`").

At this point, the user can choose between options 1 to 3 in the main
menu.

###  Main menu

 PRESS:
 1 FOR TEXT EDITOR
 2     TEXT FORMATTER
 3     UTILITY


 c)1982 TEXAS INSTRUMENTS     
 TOETS IN:
 1 VOOR BEWERKEN VAN TEKST
 2      OPNAKEN VAN TEKST
 3      HULPROGRAMMA


 c)1982 TEXAS INSTRUMENTS     

Option 1 enters the text editor.
Option 2 calls the text formatter and printer.
Option 3 allows the user to load a "program" file in EA5 format.
Fctn= resets the TI-99/4A.

####  Text editor

Option 1 first loads a file containing the characters patterns for a
given language. This file is called CHARA1 for english, CHARB1 for
french, etc. The file PAB is located at \>0680 in VDP memory, and the
data buffer area is \>07FA. Since all these files begin with a 6-byte
header (EA5 standard), the file data start at \>0800, which corresponds
to the beginning of the character pattern table. A very elegant way to
modify that table, indeed. Of course, the contents of the file is not
tranfered in cpu memory (the 6 header bytes are all zero).

The number of available files is then lowered to 1, by calling
subprogram \>16 in the disk controller card. This is the equivalent of a
CALL FILES(1) in Basic.

The program proceeds by loading an EA5 file called EDITA1 in english,
EDITB1 in french, etc. The PAB is still at \>0680, but this time the
data buffer area is located at an address taken from word \>8386, which
points to the first available address in VDP memory. In complieance with
the EA5 standard, the loading routine can continue with loading a file
called EDITx2 if the header of the first file requires it (it does).

If the loading operation fails, the program attempts to load the english
version of the editor: EDITA1 and EDITA2. An error message will be
displayed only if that second attempt is unsuccessfull.

If loading was successfull, word \>8386 is set as \>1000 (first free
address in VDP memory), VDP register 1 is set as \>30 (screen off, text
mode), the screen is erase, byte \>8384 is cleared and the editor is
entered via an `XML >F0` (i.e. the vector is in word \>8300).

The editor loads a "program" file (not in EA5 format) called TXTEA1 for
english, TXTEB1 for french, etc. This file contains the promtps used by
the editor within its menus, and the key combinations to be entered by
the user to trigger one or the other command (e.g. SD for ShowDirectory
in english, AR for AfficherRepertoire in french, etc).

Upon return, the value of byte \>8384 determines the action to be
taken:
0: Re-initialise the module.
1: Load an EA5 file whose PAB in VDP memory is pointed by word \>8389
and execute it (in case of an error, re-enter the program with
`XML >F0`).
2: List the disk directory, for the drive number found in \>8385. Once
done, re-enter the program
3: Load and runs the formatter.
Other values: Performs a GPL `RTN`.

####   Text formatter

Option 2 loads the character patterns file, just like option 1 and sets
the number of files to 4.

It then loads the EA5 files FORMA1 and FORMA2 for english, FORMB1 and
FORMB2 for french, etc. Here also, if loading fails, the program
attempts to load the english version, before to annouce an error.

The formatter is launched by the same routine than the editor, and the
same actions can be taken upon return, depending on the value in byte
\>8384.

The formater loads a "program" file (not in EA5 format) called TXTFA1
for english, TXTEF1 for french, etc. This file contains the promtps used
by the formatter to interact with the user, and the formatting commands
to be expected inside the text (e.g. LM for left margin, which is MG in
french: marge de gauche).

####  Utility

Option 3 does not load any character pattern but sets the maximum number
of files to 3 and prompts the user for a filename.

This file is loaded and run just as the editor and the formatter, wich
allows for the possibility of a pre-defined action upon return,
depending on the value of byte \>8384.

### Disk directory

 DSK1 - DISKNAME= MYDISK
 AVAILABLE= 216 USED=  92
  FILENAME  SIZE    TYPE    P
 ---------- ---- ---------- -
 TEST1/S      57 DIS/VAR 80
 TEST1/O      25 DIS/FIX 80
 MOUSE1       10 PROGRAM    Y
 COMMAND COMPLETED
 PRESS: &quot;ENTER&quot;
 DSK1 - NOM= MONDISQUE
 LIBRES= 216 UTILISES=  98
  FICHIER   DIM     TYPE    P
 ---------- ---- ---------- -
 TEST1/S      57 DIS/VAR 80
 TEST1/O      25 DIS/FIX 80
 SOURIS1      10 PROGRAM    Y
 FIN D&#39;EXECUTION
 APPUYEZ SUR &quot;ENTER&quot;

As mentionned above, this subprogram is only available when called from
the editor, or from an utility EA5 program that exits with the value
\>02 in byte \>8384 and the relevant drive number in byte \>8385. That's
especially strange since it's written in GPL: it wouldn't cost much to
create an adequate entry point from the main menu. Maybe that was
deliberate from Texas Instruments, so that the cartridge won't be used
without the floppy?

When entered, the program loads VDP registers 1 to 6 with the values
stored in GROM, and disables the sprites. It initialises the color table
with the colors placed in byte \>83FB by the calling program. Then is
displays "Catalog Disk" and "Press clear to abort" in the appropriate
language.

To display the directory, it calls subprogram \>10 from the disk
controller card: this is the sector read/write low level utility. It is
used to read sector 0 into a buffer located at \>0400 in VDP memory.
Data in sector 0 is used to display the disk name, and to calculate the
number of free and available sectors (the program counts the "1" bits in
the bitmap).

Then it calls subprogram \>10 again to read sector 1, that contains a
list of file pointers. Again, the data buffer is at \>0400. The program
then walks the list of file pointers until it reaches a \>0000. For each
file it loads the corresponding File Descriptor Record (using subprogram
\>10) into a buffer located at \>0500 in VDP memory. The relevant
informations are extracted from the FDR and displayed on screen. If
necessary the screen scrolls up, but never overwrites the title lines.

The user can press the spacebar to temporarily pause and resume the
listing. Pressing Fctn-4 aborts the operation. Whether aborted or
normally completed, the program then displays "Command completed",
"Press: enter" and waits for the user to press \.

It then clears the screen, loads VDP registers 1 to 6 with the values
taken from bytes \>83B9-83BE (placed by the calling program in the GPL
data stack) and re-enters the calling program with the same `XML >F0`
that was used to launch it (i.e. \>8300 should contain the address of
the entry point).
Revision 1. 6/22/99 Ok to release

[Back to the TI-99/4A Tech Pages](titechpages.md)
