# EEPROM loader & manager for the USB-SM card

This page describes ELM ("EEPROM Loader & Manager"), a program meant to
manage the EEPROM on the USB-SM card. Its main function is to load new
DSRs into the EEPROM, but it also lets you inspect the EEPROM, edit it,
and possibly remove old DSR.

There are 3 level at wich the progem can be used:

1.  As a DSR loader, to load a single DF80 file.
2.  As an EEPROM manager, to enter commands and view their results.
3.  Or you can feed it a script file, in DV80 format, containing a
    pre-arranged series of commands and/or files to be loaded.

The distibution disk can be downloaded [here](elm.zip) (zip file, 58K),
in disk-image format, i.e. a sector dump in numerical order. To transfer
it to your TI-99/4A, see my [download](download.md) page.

The disk contains the following files:

    ELM/1 to ELM/7     Prog   EEPROM manager and loader
    MENU               DV80   Menu driven interface for ELM
    MENU_B, MENU_E     DV80   Submenus called by the above
    STDMEM/O           DF80   Standard memory access routines
    USBHOST/O          DF80   Demo DSR, to access USB devices from Basic
    USBHOST            DV80   Script loading the above two files
    DIRECT             Prog   Basic demo program
    GEDESC             Prog   Basic demo program
    HOST               Prog   Basic demo program
    INTS               Prog   Basic demo program
    PORTS              Prog   Basic demo program

[Loading a new DSR
](#Simple%20load)[Command mode
](#Command%20mode)[Demo DSR](#USBHOST%20demo)

## Loading new DSRs

This is the simplest way to use the EEPROM manager. Before you start,
make sure that all switches on your USB-SM card are in the proper
position: the one on the back should be all the way back with a TI-99/4A
or in the middle position for a Geneve. The switch in the middle of the
board should be forward with a TI-99/4A, and to the back with a Geneve.
In addition, you should close the top switch (labelled EE) on the
DIP-switch located to the front of the board. This will unprotect the
EEPROM and allow the program to write DSR into it. You may want to open
this switch once you are done: this will prevent any malicious program
(e.g. a virus) to install itself permanently in your USB-SM card.

Load the file ELM/1 with a EA5 loader such as Funelweb, the
Editor/Assembler cartridge option 5, or the TI-writer cartridge option
3.

The program automatically scans for the USB-SM card and displays its CRU
address and the size of its EEPROM.

At this point, just type in the name of the file you wish to load. In
most cases, it will be a DF80 file, but it could also be a DV80 script
file.

The program then prescan the file and displays its info screen, if it
contains one. Press any key to continue with loading the DSRs, or press
Fctn-= (QUIT) if you realize this is not the file you wanted. If the
file does not contain any information for the user, you don't have the
option to abort loading. Neither do you when you are running a script,
unless the author of the script chose to give you such an option.

#### Notes

If you have more than one USB-SM card, several CRU addresses will be
displayed. The last one will be used by default. If that's not the one
you want, type in "CRU 1x00", where x is the address you want.

If the USB-SM card is not found, most probably the main switch was left
in the "off" position (or the TI/Geneve switch is wrong). Correct the
problem then type in "CRU" to repeat scanning. Or just restart the
program.

------------------------------------------------------------------------

## Command mode

Load the file EEMAN1 with an EA5 loader. You then have two
possibilities:

If you prefer a menu-driven interface, type in DSK1.MENU (or whatever
drive the files MENU, MENU_E, MENU_B are located in). This will launch a
chain of scripts that let you access most (but not all) commands through
a series of menus. The script files are in DF80 format and can thus be
read with a text editor. If you need examples of how to use commands,
you may want to take a look at these files.

If you like a command line interface better, just type in commands, in
upper-case. Many commands can be abbreviated by just typing their first
two letters (see command summary). The following function keys are
active: Fctn-3 erases the line, Fctn-S backspaces.

Commands can be divided into several groups:
[file-oriented](#file%20commands) commands,
[EEPROM-oriented](#EEPROM%20commands) commands,
[debugging](#debug%20commands) commands, and
[script-file](#Script%20commands) commands. A [command
summary](#Command%20summary) is provided at the end of this chapter.

Most commands produce screen output, so you know they are executed
correctly, but there are a few silent commands. As long as you don't get
an error message, you can assume that the command was executed properly.

If an error occurs, you may have the option to ignore it and continue
with the command, at your own risks. This is not possible with every
command, though. If you are executing a script, you are also given the
option to abort the script and return to command mode. Errors that
report duplicate names (e.g. duplicate DEF, or duplicate DSR or call)
give you the option to replace the old entry with the new one, or to
keep the old one and just ignore the new one.

To exit the program from the command line interface, just type EXIT

#### Entering numeric values

Many commands take numeric values as parameters. All numeric values are
assumed to be in hexadecimal format, so there is no need for the \>
mark, although it is accepted. If you wish to enter a decimal number,
you must precede it with a \# sign. In other words, \#10 means ten,
whereas \>10 and 10 both mean sixteen.

Wherever a numeric value is required, you can substitue a symbol name
preceded with an @ at sign (e.g. @TEST). The value of this symbol will
be used, if is is found in the symbol table. If the symbol is not found,
the value is zero by default. Several internal variables used by the
loader can be accessed in this way, see [table
below](#Predefined%20symbols). (In some cases, you can also use labels
where a text value is required, in wich case the label should contain a
pointer to a string).

You can combine numeric values and symbols in mathematical expressions,
using the following operators:

Unary operators, act upon the value that follows:
+ ignored
- negation (-1 is FFFF)
~ bitwise inversion (~1 is FFFE)
\` absolute value (this is the "accent grave": Fctn-C)

Dual operators, combine two values:
+ addition
- substraction
\* multiplication
/ division
% modulo (remainder of division)
\] the greatest of two (signed comparison)
\[ the smallest of two (signed comparison)
& bitwise "and"
\| bitwise "or"
^ bitwise "exclusive or"

Unary operations are performed first. Dual operations are performed in
order of appearance. To alter the order of execution you can use
parentheses ( ) , which can be nested upto 5 levels deep. Parentheses
mismatch or overnesting abort the calculations. As a general rule, any
syntax error aborts calculations and returns the current value,
calculated upto the point where the error occured. Note that no check is
performed for carry, borrow, nor overflow during arithmetic operations.

Examples:

- EQUATE THIS, 22 & ~16       ; THIS will be >0020
      EQUATE THAT,2 * @THIS+5     ; THAT will be >0045
      EQUATE TEST, @THIS [ #31    ; TEST will be >001F (i.e. 31 decimal)
      EQ TEST, 55 ] 61 ] FFFC     ; TEST will be >0061
      EQ TEST, ~(-2)              ; unary ops cannot follow each other without parentheses

###  File loading commands

These commands are used to load DSRs from object files and "burn" them
(i.e. write them) into the EEPROM.

[LOAD
](#LOAD)[BURN
](#BURN)[RESET
](#Reset)[INFO
](#INFO)[ENTRIES
](#ENTRIES)[EXPORTED
](#Exported)[EXIT](#EXIT)

#### LOAD command

This command causes a DF80 tagged-object file to be pre-scanned. The
loader looks for the segment structure within the file (tags M) which it
uses to build the segment table, and for DEFs and REFs which it places
into the symbol table (unless a DEF has the same name than a segment, in
which case it is assumed to contain segment flags for the segment
table).

Once it is done reading the file, the loader calls its dispatcher
routine and attempts to assign each segment to a page within the EEPROM.

If everything goes well, you are now ready to burn segments into the
EEPROM, or to review file contents with commands like INFO, EXPORTED, or
ENTRIES.

The LOAD command can also be used to trigger execution of a DV80 [script
file](#Script%20commands), or to chain script files if enclosed within a
script (note that there is no mechanism to return to the same point in
the calling file once the chained file is finished).

Normally, the command should be followed with a file path, i.e. a drive
name and a file name: DSK2.MYFILE. But there are several alternative
syntaxes. If the file path begins with a decimal point, the default path
name (DSK1.) will be substituted for the dot. If the path begins with
DSK\*, the number of the most recently used drive will be substituted
for the star. Finally, you could use a label pointing to a string, which
should contain the file path (e.g. LOAD @\_PTR).

####  BURN command

BURN

This command causes the DF80 file to be read again, once for each
segment, starting from the end of the file. The segments are assembled
one by one into the memory expansion card, then written into the EEPROM
(a.k.a. "burned", a term that dates from the time PROMs were made of
tiny fuses that were burnt to write a '0' bit).
BURN *segment_number  *
BURN "*segment_name*"

Alternatively, you can use the command to write only one segment into
the EEPROM, by specifying either its number or its name enclosed in
doubled quotes. Note that the loader will need to assemble in RAM all
segments following the one you're burning, for proper resolution of the
REF chains. So if you're planning to `BURN `more than one segment, it is
best to burn them in reverse order of appearance in the file (i.e. start
with the last one).

####  RESET command

RESET

This command wipes out the segment table, the symbol table and restores
several values to their default. It is approximately similar to exiting
the program and restarting it, except that you don't loose the CRU
address, nor some of the debugging settings.

If you wish to load more than one DSR unit in one session, make sure you
insert a RESET after each unit has been burnt.

There is a special syntax of this command:

RESET SEGS

which only resets segment-related info, but preserves the symbol table.

####  INFO command

This command is used to display the information segment in the DF80
file, if there is one. This segment will never be loaded, it is only
meant to provide information about the contents of the DSRs.

There are 4 records in this segment: a target identifier (which should
say USB-SM), an info line, an info screen, and a custom info field.

If the info segment contains a screenful of info, it is displayed until
you press a key. Then the target name and the info line (if any) are
displayed and the command ends.

####  ENTRIES command

This commands loads the blank (no-name) segment from the DF80 file, and
displays how many of each entry points it contains: DSRs, CALLs,
power-up and interrupt service routines. It also verifies that there is
enough room in page 0 to load this segment.

    ENTRIES

    DSRs: 0002    Calls: 0001
    Pwr-up: none  ISR: none
    Will not fit in page 0

To get detailed information on the entries about to be created, you can
follow the command with a parameter: DSR, CALL, ISR, POWERUP, or ALL
(only the first letter is scanned). The relevant entries are listed with
the following format:

    ENTRIES ALL

    ? Name    Addr Dup
    - ------  ---- ---
    D NEWDSR  412E  Y
    D OTHER   4132
    C CALL4   4144
    p         4160
    i         415E
    Will not fit in page 0

- The first column displays the type of entry: "D" for DSRs, "C" for
  calls, "i" for sub-ISRs (no master ISR or power-up can be defined by
  the entry segment), and "p" for sub-powerup routines.
- The second column displays the name of the entry, for DSRs and CALLs.
  It is left empty for ISRs and power-up routines which have no name.
- The third column displays what will be the entry point of the routine
  after it is installed inn the EEPROM
- The last column displays a "Y" if the name of this entry already
  exists in the EEPROM header.

Since the entry segment is generally burned last, it is a good idea to
call this command before calling BURN, in order to detect potential
problems before anything is commited to the EEPROM.

After execution of an ENTRIES command comprising a parameter, \_ITEMS
will contain the number of items listed. ENTRIES without parameters sets
\_ITEMS as zero.

####  EXPORTED command

This command loads the EXPORT segment from the DF80 file (if any) and
displays it on screen. It also checks the exports list whithin the
EEPROM and makes sure there are no duplicate names.

    Name   Addr Dup Segment Page
    ------ ---- --- ------- ----
    MALLOC 46DA     STDMEM  0004
    FREE   46DE     STDMEM  0004
    RAMCPY 46E2     STDMEM  0004
    BURNW  46E6     STDMEM  0004
    TOSLOT 46EA     STDMEM  0004
    ATSLOT 46EE     STDMEM  0004
    VSBR   49AA  Y  STDMEM  0004
    VSBW   49EA  Y  STDMEM  0004
    VMBR   49B2  Y  STDMEM  0004
    VMBW   49B6  Y  STDMEM  0004
    VWTR   49BA  Y  STDMEM  0004
    VIBW   49BE     STDMEM  0004

After execution, \_ITEMS will contain the number of exports found in the
EXPORT segment.

####  EXIT command

This command simply exits the program and resets the TI-99/4A. Whatever
EEPROM page that was selected last will appear in the EEPROM. For your
DSRs to work properly, this should be page 0 (or the one you substituted
for it with the PAGE0 command). To force page 0 selection, just turn the
console power off, then on.

###  Analyzing the EEPROM

There are several commands dedicated to the analysis of the data
currently within the EEPROM.

[VIEW
](#View)[PAGE
](#PAGE)[HEADER
](#Header)[UNIT
](#Unit)[EXPORTS
](#Exports)[ERASE
](#Erase)[PATCH
SCRATCH](#SCRATCH)

####  VIEW command

This is the most basic command: it just displays the contents of the
EEPROM. There are several syntaxes:

VIEW *address:page*

This version of the command displays 128 bytes of data, arranged as 16
lines of 8 bytes.

    VIEW 4000:1

    4000 50 00 00 01 4D 41 4C 4C
    4008 4F 43 41 80 00 03 46 52
    4010 45 45 20 20 41 84 00 03
    4018 52 41 4D 43 50 59 41 88
                 etc.
***Address*** is the address where the display should start. Normally,
it will be in the range \>4000-5FFF, but this is no obligation: you can
use VIEW to display any cpu memory area.

***Page*** is the number of the EEPROM page, if address is within \>4000
to \>4FFF. Otherwise, it is assumed to be the number of the SRAM page
(the SRAM maps at \>5000-5FFF). The maximum number of EEPROM pages is
displayed when starting the program: it's either \>0800 for an 8 meg
EEPROM, or \>0400 for a 4 meg EEPROM. The maximum number of SRAM pages
is \>0100.
VIEW *address:page,ASCII*

This is the same as above, but the presence of the ASCII flag (only
letter A is checked for) turns the display into text. There will still
be only 8 bytes per line, at the same positions as the hexadecimal bytes
were, so you can figure out more easily which is which. Unprintable
characters are displayed as double underscore, which lets you
differentiate them from the regulr underscore character (ascii 95) which
appears only once.

    VIEW 4000:1,A

    4000  P  `  `  `  M  A  L  L
    4008  O  C  A  `  `  `  F  R
    4010  E  E        A  `  `  `
    4018  R  A  M  C  P  Y  A  `

VIEW *address*

This displays another address from the same page, in hexadecimal or
ascii, depending on what was the case the last time the command was
typed in.
VIEW

This diplays the next 128 bytes, in the same format as previously
VIEW -

The displays the previous 128 bytes.

After execution, \_PAGE contains the number of the displayed page, and
\_PTR the starting address of the displayed memory.

####  PAGE command

This command lets you analyse EEPROM pages. It works best if pages in
use contain the default link structure installed by the loader, in which
each segment is preceded with a pointer to the next segment. However, it
can also provide information about non-standard pages.

There are 3 disting syntaxes, differing by the number of parameters. The
simpler version, PAGE with no parameters, displays a summary of EEPROM
pages:

    PAGE

    Free: 07F8  Bytes: 7F98A6
    Used: 0008  Segments: 000C
    Content Page Items Free
    ------- ---- ----- ----
    Exports 0001 000B  0FE8
    Units   0002 0003  0F7E

**Free** is the number of free pages, followed with the number of free
bytes. Each free page contributes \>1000 bytes to the total, to which
the program adds any free bytes remaining at the end of used pages. Note
that, due to time constraints, the program does not actually verifies
that all 'free' bytes contain \>FF, it just trusts the link structure
installed in used pages.

**Used** is the number of pages in use, either fully or in part.
**Segments** indicates how many segments are loaded into these pages.
This number includes all stubs, so it may be greater that what is
displayed with the UNIT command.

Then the program lists all export pages and all "unit" pages, i.e. those
containing removal information. For each page, its number is displayed
together with the number of exports or units in the page, and the
remaining free bytes at the end of the page.

After execution, \_ITEMS contains the number of free pages. \_PAGE
contains the number of the first units page.
PAGE *first,last,flag*

You can get a more detailed listing by following the command with a
range of pages.

***First*** is the first page to be listed.
***Last*** is the last page included into the listing.
***Flag*** is an optional condition: if it is USED (or just the letter
U) only used pages will be listed. If it is any other value (such as
FREE) only free pages will be listed. If it is omitted, all pages will
be listed.

    PAGE 0,6

    Page  Segs  Used Blank
    ----  ----  ---- -----
    0000: 0000  0000 Blank
    0001: 0001  1000 0EB2
    0002: 0001  1000 0FA8
    0003: 0003  0C32 05C4
    0004: 0002  06D8 0B1E
    0005: 0000  0000 Blank
    0006: 0000  0000 Blank

- The first column is the page number.
- The second column is the number of segments found in this page,
  including the stub (if any).
- The third column is the number of bytes marked as used (i.e. part of a
  segment) within the page.
- The last column is the number of \>FF bytes at the end of the page
  (whether marked as used or not).

After execution, \_ITEMS contains the total number of pages listed, i.e.
matching the flag, \_PAGE contains the number of the last page listed.
PAGE *number*

To get even more details on the contents of a page, you can use the PAGE
command with a single argument, the page number. The following type of
listing is produced:

    Addr Size
    ---- ----
    4000 017E Default stub
    417E 055A
    46D8 0558
    4C30 03D0 Blank

- The first column is the address of the segment.
- The second column is the segment size.
- The third column may contain extra info on the segment

The first segment in a page is systematically compared with the default
stub used by the loader. If it is identical, this will be indicated in
the third column. If it matches the default stub but uses more space,
stub extensions were included, which is indicated by a '+' sign.

After the last segment, one more line is printed with the number of
blank bytes (i.e. containing \>FF) at the end of the page. If the last
segment finishes with \>FF bytes, they will be included in the count, so
the number of blanks may be greater than expected. If the last segment
reserved the full page, the number of blank bytes at the end of the page
will be indicated on the segment line.

    Addr Size
    ---- ----
    4000 1000 Blank: 0EB2

If a segment was loaded without using the default chaining mechanism
(i.e. preceding it with a pointer to the next segment), its size cannot
be calculated and "no link" is indicated instead. Since the next segment
cannot be found either, the routine just displays the number of blank
bytes at the end of the page.

    Addr Size
    ---- ----
    4000 No link  Blank: 0002

After execution, \_ITEMS contains the number of segments listed. This
includes the strub segment, but not the blank portion at the end of the
page.

####  HEADER command

This command analyses the EEPROM header to be found in page 0. It
expects byte \>4000 to be \>AA, and further checks address \>4010 to
know whether the current header is the default one, installed by the
loader.

It then analyses the header and displays the following statistics:
number of DSR names, number of subprogram (i.e. CALL) names, number of
power-up and interrupt routines. If the header is the default one,
power-up and ISRs are broken into two categories: the master routine is
the one called by the console ROMs, the subroutines are called by the
master. Normally, there will be only one master routine in each
category, since the loader only installs subroutines.

If the header is the default one, the command also displays the location
of the first exports page, and that of the first "units"
page, i.e. the page containing removal informations. Finally, it also
displays the number of entry segments in page 0, normally at least one
since the default page 0 stub is counted as one.

    DSRs: 0003 Calls: 0002
    Master PwrUp: 0001 Sub: 0000
    Master ISR:   0001 Sub: 0000
    Exports page: 0001 Units: 0002
    Entry segments: 0002

After execution, \_ITEMS contains the number of entry segments found.
\_PTR points to the first free address in page 0.

You can further refine the analysis, by including after the command the
category that you want to be detailed: DSRs, CALLs, ISRs, Power-ups.
Only the first letter is checked, so feel free to abbreviate these as D,
C, I and P.

    HEADER DSR

    Name   Addr Unit (Hex name)
    ------ ---- ---- ---------
     YDSR  4120 0001 05005944
    MYDSR  4340 0002
           6000      0000

- The first column list DSRs (or CALLs) by name, whether printable or
  not.
- The second column is the entry point in page 0 for this DSR.
- The third column is the loading unit this DSR belongs to.
- If the first character in the name is not printable, the last column
  displays the size and name of the DSR in hexadecimal format. Due to
  lack of space, no more than 3 characters can be spelled out this way.

In the exemple above, the first DSR was scratched, by zeroing the first
byte in its name. Most probably because unit 2 wanted to define a DSR of
its own, with the same name. \>05 is the name size, \>00 the erased
character, \>59 and \>44 the next two characters ("YD").

There is always an extra entry at the end of the list, don't worry about
it. It is a dummy name residing at \>4FFE, which allows the loader to
extend the chain of names when new DSRs are loaded (\>4FFE can be
transformed to any address by changing zeros ro ones).

The listing for calls, obtained with `HEADER CALL`, follows the same
format.

However, ISRs and power-ups have no name, so their listing is a bit
differnent.

    HEADER ISR

    T Addr Unit
    - ---- ----
    M 4020
    i 4192 0001
    i 41AC 0002

- The first column indicates whether it is a master ISR (or power-up) or
  a subroutine. A letter "M" indicates a master routine.
- The second column is the entry point of the routine.
- The third column is the number of the unit it belongs to.

The listing is identical with `HEADER PWRUP` which lists power-up
routines.

After execution, \_ITEMS contains the total number of items displayed.

####  UNIT command

This command lets you inspect what DSRs where previously loaded into the
EEPROM. Each time it installs a new DSR, the loader saves some
information about it into a dedicated page(s). This information will
come handy in case you need to remove a DSR. Each loading session
contributes an entry into this page, hereafter known as a "unit".

By typing the UNIT command without any parameter, you get a summary
table of all registered units.

    UNIT

    Unit  Seg  Exp  Ref  Entry
    ----  ---- ---- ---- -----
    0001: 0001 0000 0000 Y
    0002- 0002 000B 0000 n
    0003: 0006 0000 0007 n

- The first column is the unit number. It is followed with a colon,
  unless the unit was deleted (with the SCRATCH command), in which case
  a '-' sign is used instead.
- The second column indicates the number of segments loaded by this unit
  (not including stubs).
- The third column is the number of exports defined by this unit.
- The fourth column is the number of exports refered to by this unit.
- The last column indicates whether or not the unit installed an entry
  segment in page 0, i.e. if it defined DSRs, calls, ISRs or power-up
  routines.

After execution, \_ITEMS contains the number of units listed. \_PAGE
contains the number of the last page with unit information in it, and
\_PTR points to the last unit entry in this page.
UNIT *number*

You can get more details about a unit by typing its number after the
command. The command displays a series of tables, inasmuch as they are
relevant for the unit in question:

- Segment list
- Export list
- Reference (to exports) list
- DSRs list
- CALL list
- ISR list
- Power-up routine list

If you do not wish to see all this information, you can display only one
of these lists by typing its name (the first character is enough) after
the unit number.

    UNIT n,SEG

    Page Addr Size
    ---- ---- ----
    FFFF 5040 0000
    0000 0000 0000
    0003 4148 0558

- The first column is the page in which the segment is located.
- The second column is the segment address in this page.
- The third column is the size of the segment.

Note that the first two segments are always listed even if empty: the
data segment (located in the SRAM, so the page \# is irrelevant) and the
"no-name" entry segment.

    UNIT n,EXPORT

    Name   Addr Page Unit
    ------ ---- ---- ----
    MALLOC 4180 0003
    FREE   4184 0003
    RAMCPY 4188 0003

- The first column is the name of the export.
- The second column is the address of the exported routine.
- The third column is the page in which the exported routine resides.
- The last column is not used here, since all exports will be in the
  same unit: the one you typed in.



    UNIT n,REF

    Name   Addr Page Unit
    ------ ---- ---- ----
    ATSLOT 4194 0003 0001
    TOSLOT 4190 0003 0001
    MALLOC 4180 0003 0001
    FREE   4184 0003 0001

- The first column is the name of the referenced export.
- The second column is the address of the exported routine, as found in
  the EEPROM exports list.
- The third column is the page in which the exported routine resides.
- The last column is the unit which exported this routine.



    UNIT n,DSR

    Name   Addr Unit (Hex name)
    ------ ---- ---- ---------
    MYDSR  4340

- The first column list DSRs (or CALLs) by name, whether printable or
  not.
- The second column is the entry point in page 0 for this DSR.
- The third column is the loading unit this DSR belongs to, i.e. the one
  you typed in.
- If the first character in the name is not printable, the last column
  displays the size and name of the DSR in hexadecimal format. Due to
  lack of space, no more than 3 characters can be spelled out this way.

The display for `UNIT n,CALL` is identical

    UNIT n,I

    T Addr Unit
    - ---- ----
    i 4192 0001
    i 41AC 0002

- The first column indicates whether it is a master ISR (or power-up) or
  a subroutine. A letter "M" would indicate a master routine, but since
  these do not belong to any unit (they are in the stub) you should
  never see one.
- The second column is the entry point of the routine.
- The third column is the number of the unit it belongs to.

The listing is identical with `UNIT n,PWRUP` which lists power-up
routines.

After execution, \_ITEMS contains the total number of items displayed.
\_PAGE contains the number of the information page where this unit is
listed, \_PTR the address of the unit entry in this page.

####  EXPORTS command

This command displays the exported symbols stored within the EEPROM, if
any.

EXPORTS *first,total*

**First** specifies the number of the first export to display, counting
from zero. If ommited, the default is zero.
**Total** specifies the total number of exports to list. If ommited, the
list is processed upto its end.

    EXPORTS 0,3

    Name   Addr Page Unit
    ------ ---- ---- ----
    MALLOC 4180 0003 0001
    FREE   4184 0003 0001
    RAMCPY 4188 0003 0001

- The first column is the name of the exported symbol.
- The second column is the address of the exported routine.
- The third column is the page in which the exported routine resides.
- The last column is the unit which exported this symbol.

After execution, \_ITEMS contains the total number of exports that were
displayed. \_PAGE contains the number of the page in which the last
listed export is located, and \_PTR points just after this export entry.

####  ERASE command

This command is used to erase the EEPROM, i.e. to reset all bytes to
\>FF, which will allow you to load new DSRs afresh. Erasure occurs by
blocks of 128 pages, so you have the option to erase only one block, or
the whole chip.

To erase one block, follow the command with the number of any page
within this block:

    ERASE 21 ; erases pages 32 to 63 (>21 hex is 33 decimal)

To erase the whole chip, just type:

    ERASE ALL

Be aware that a whole-chip erase takes almost a minute to complete.

After the command is completed, \_ITEMS contains the number of blocks
erased.

####  PATCH command

With this command, you can modify the content of the EEPROM. But
remember that you can only change '1' bits into '0', not the opposite
(for this you would need to erase the EEPROM).

PATCH *address:page,byte,(byte,...,byte)*

The command is followed with an address and an EEPROM page. The address
must be in the range \>4000-4FFF, do not use PATCH to edit any other
memory area, it will not work.

The page number is followed with one or more bytes to be written into
the EEPROM. The upper limit is the maximum number of bytes that you can
write in a single input line.

`PATCH 4028:2,20,00 ; This writes >2000 at address >4028, in page 2`

If everything goes well, you are returned to the command prompt. If an
error occurs, it is most likely because the write-protection switch was
left open, or because the bytes to write would turn 0s into 1s (e.g if
\>4028 contains \>1234, you can't change the '1' into a '2').

####  SCRATCH command

This command can be used to remove an entry, an export, or a whole unit
from the EEPROM. This is useful if you are planning to load a new
version of a DSR, which defines the same names as the old one. Note that
the items are not actually removed, but rather the first character in
their name is zeroed.

You can scratch DSRs and Calls by name, ISRs and power-up routines by
entry address, and exports by name. You can also scratch a whole unit,
which sets a flag into the unit information table and scratches all
exports and entries that belong to this unit.

The various syntaxes are thus:

    SCRATCH DSR,THIS

Removes the DSR named "THIS", by turning the T into a \>00 byte.

    SCRATCH CALL,MINE

Ditto, scratches the call named "MINE".

    SCRATCH, ISR,412E

Scratches the ISR with an entry point at \>412E in page 0. This is the
address where execution begins, not the address of the entry in the ISR
chain. The ISR is scratched by changing its entry word to \>0000 (the
master ISR knows to ignore such entries). The ISR subroutine itself is
not altered.

    SCRATCH PWRUP,4410

Ditto, scratches the power-up routine which begins execution at address
\>4410, by zeroing its address in the power-up subchain.

    SCRATCH UNIT,2

Scratches all entries and exports for the second unit found in the unit
removal info pages. It also sets a flag in the unit's entry that marks
it as removed.

Note that category names can be abbreviated upto a single letter: D, C,
I, P, E and U.

After scratching a DSR or a call, \_PTR points to its entry (i.e. a
"next-link" word, followed with the execution address, a size byte for
the name, and the name itself), and \_PAGE will be zero. The same thing
happens after scratching an ISR or a power-up routine, except that these
entries have no name. After scratching an export, \_PAGE contains the
number of the page where this export was found, and \_PTR points at its
entry (i.e. a 6-character name, followed with an address and a page
number). After scratching a unit, \_PAGE contains the number of the unit
information page, \_PTR points at the unit's entry in this page, and
\_ITEMS contains the total number of exports, DSRs, calls, ISRs, and
power-ups that were scratched. If the item to erase was not found, \_PTR
contains zero.

###  Debugging commands

The following commands are meant for people who write their own DSRs.
They allow you to tweak the contents of the memory and to alter the
behaviour of the loader, so you can test your DSRs better.

[CRU
](#CRU)[SYMTAB
](#Symtab)[EQUATE
](#Equate)[NEWBLOCK
](#NEWBLOCK)[PAGE0
](#Page0)[SEGTAB
](#Segtab)[ASSIGN
DISPATCH
](#Assign)[PG4SEG](#PG4SEG)

####  CRU command

This command is used to change the CRU of the target card. Normally, the
CRU is assigned automatically, as the program scans for the USB-SM card
upon start up. However, if you have more than one card in your system,
you can use this command to select one or the other. The command also
queries the EEPROM and displays its size in pages.

CRU \>1E00

At any time, you can access the symbol \_CRU to find out what's the CRU
currently in use, and \_LSTPG for the number of the last EEPROM page.
You could change \_CRU with an EQUATE statment, but then there would be
no check for the presence of the EEPROM...

####  SYMTAB command

This command displays the loader's symbol table. Entries in this table
come from the REF and DEF tags included within the DF80 file(s) loaded,
and from any symbol you defined with the EQUATE command. Exports are not
part of the symbol table.

You can affect the listing by specifying which type of symbol you would
like to see: DEF, REF, or UNDEFINED (only the first letter is checked).
Symbols defined with EQUATE are considered as DEFs. If you do not
specify any filter, all symbols will be listed.

    Name   Value Seg Type
    ------ ---- ---- ----
    MYDEF  407C 0002 D
    MYREF1 4080 0002 U
    MYREF2 4090 0002 R

Note that after a LOAD, all REF labels are still undefined. Only after
BURN, will they appear as "R".

After execution, \_ITEMS contains the number of symbols that were
effectively listed (i.e. matched the required type, if any), and \_PTR
point to the symbol table. Note that the table grows backwards, toward
lower memory addresses.

####  EQUATE command

This command lets you assign a value to a symbol, creating the symbol if
it does not already exist. There are many possible syntaxes.

EQUATE *symbol,value:segment*

This one assigns a numeric value to the specified symbol:

    EQUATE MYLABEL,>1234

Optionally, you can also specify the segment number, using a : sign as a
separator:

    EQUATE MYLABEL,1234:000C

Valid segment numbers are 0 (the default) through 127. Values greater
than 256 have special meanings: \>8xxx is for unresolved refs, \>FFFF
for solved REFs, \>2400 for GOTO labels, etc.
EQUATE *symbol*,WORD,*address:page*

This version of the command will read a word from the specified address
(evened out) at the specified page and assign it to the symbol. If the
page number is not specified, the current page is used.
EQUATE *symbol*,BYTE,*address:page*

This does the same, except that only one byte is read, so the symbol
value will always be smaller than 256. The address may be uneven.
EQUATE *symbol*,SEGMENT,"*segment_name*"

The double quotes around the segment name are optional. The symbol will
contain the number of the segment, if it is found in the symbol tabel.
In addition, \_PTR will point to the segment entry in the segment table
(a 6-char name, followed with page, address, size, flags, and 1 reserved
word). If the segment it not found, the symbol value remains unchanged,
and \_PTR will be zero.
EQUATE *symbol*,\$

This special syntax lets you assign the current location in a script
file to a symbol This way you can create labels where to jump within the
script file, with a GOTO statement. Obviously, this command only makes
sense within a script.

Notes:
Expressions like \$+12 are ignored: anything begining with \$ stands for
the current script line (or more exactly, the address of this line in
the VDP memory).
If you display the symbol table, you will notice that the "segment
number" is set as \>2400 for this kind of label (\>24 is the ascii code
for \$).

After execution, \_PTR points at the symbol entry in the symbol table,
which consists in the 6-character symbol name, the symbol value, and the
symbol's segment number. The exception is for segment, for which \_PTR
points at the segment's entry in the segment table.

Tip: to obtain pointers to DSRs, CALLs or EXPORTs, use an IF IN
statement, and get the pointer from \_PTR.

####  NEWBLOCK command

With this command you can cause the loader to pick a different block for
loading a new file, rather than the block currently in use. This will
allow you to erase the newly loaded DSRs, should they prove buggy,
without affecting those that already were in the EEPROM. There are two
possible syntaxes

NEWBLOCK page

This version of the command lets you specify a page number. From now on,
the dispatching routine will try to fit segments into this page first,
then into the following pages, but will not consider any page before
this one. The only exceptions being page 0, used for the entry segment,
and the dedicated pages used for exports and units removal info. These
can be altered with the PAGE0 command.
NEWBLOCK

When used without a page number, the command will scan the EEPROM for an
empty block, i.e. 32 consecutive blank pages starting on a multiple of
32 (but not zero). If it finds one, the first page in the block will be
the point where subsequent loading will begin.

After execution, the dedicated symbol \_FSTPG will contain the number of
the selected page, which is also displayed onscreen. The default upon
starting the program is \>0003.

####  PAGE0 command

With this command you can substitute another page for page 0. The loader
will place the entry segment into this page, instead of page 0. If you
so wish, you can also specify different pages for the export list and
the removal information (units) list. This will allow you to test your
DSRs without disturbing those that are already in place. It is
especially usefull to select a page located within a currently unused
block: once you are done, you can just erase the block. This can be done
by typing `PAGE0 @_FSTPG` just after calling the NEWBLOCK command.

The syntax is very simple:

    PAGE0 pg0,expg,unitpg

**pg0** is the number of the page that you would like to substitute for
page 0. The no-name entry segment (\$BLANK in the segment table) will be
loaded in this page, together with a special stub. If this number is
missing, the current page 0 specified in symbol \_PAGE0, will be
selected.

**expg** is the number of the first page to be used by the export table,
i.e. labels defined by loaded programs. This parameter is optional, as
you may elect to keep using the same export table that is currently in
use.

**unitpg** is the number of the first page to be used to store unit
removal information, i.e. a description of each program that is loaded
into the EEPROM. Again, it is optional and you may choose to keep the
current one.

These three pages must be distinct. If either *expg* or *unitpg* is the
same as *pg0*, it will be incremented by one. Similarly, if *unitpg* and
*expg* are identical, *unitpg* will be incremented.

Be aware that the page numbers for exports and units are saved into the
special stub installed in page 0. So, once you have loaded anything in
the page 0 of your choice (which automatically installs the stub), you
cannot alter *expg* nor *unitpg* any longer. But you can alwats pick
another *pg0*, with no preassigned *expg* nor *unitpg* yet. The command
always displays which pages are to be used for exports and units: if
they don't match what you just typed, you'll know it's because their
value was preset in page 0.

At any time, you may access the symbols \_PAGE0 to find out which is the
current page 0, \_EXPG for the first exports page, and \_UNIPG for the
first units page. Please do not modify them directly as no sanity check
would be done. Be aware that, if the export list or the unit info table
grows really big, another page may be added to the existing one. The
number of this page will be written at the end of the previous one,
after a \>0000 mark.

To properly test your DSR, retype the PAGE0 command (without any number)
just before you exit the program. This will ensure that your "dummy"
page 0 will be present in memory after the console has rebooted and
while it's rebooting (so you can test your power-up routines). Note that
if you turn the console off then on the real page 0 will be selected,
which is a good thing in case one of your routines would lock-up the
computer...

####  SEGTAB command

This command displays the loader's segment table. Entries in this table
come from the segment structure of the DF80 file(s), at least for the
segment name, its size and any possible flags. After a file is loaded, a
displatcher routine scans the EEPROM and attempts to fit all segments
into it. The resulting addresses and page number are placed into the
segment table. If you used the LOAD command, nothing is actually written
into the EEPROM until you issue the BURN command. If you typed the
filename directly, an implicit "load & burn" is executed, so SEGTAB
shows you what has already happened.

    Name   Page Addr Size Flags
    ------ ---- ---- ---- ----
    $DATA  0000 5040 0140 0000
    $BLANK 0000 0000 0000 0000
    MYSEG  0005 4682 032E 0000
    OTHER  0004 4102 0886 0000

Note that the data segment and the entry segment ( BLANK) are always
displayed, even if they are not used.

After this command, \_ITEMS will contain the number of segments
(including data and blank). \_PTR will point to the beginning of the
segment table.

####  ASSIGN command

This command lets you modify the segment table after a LOAD, i.e.
determine where a segment will be loaded upon execution of the BURN
command.

ASSIGN "*segment_name*"*,page,address,size,flags  *
ASSIGN *segment_number,page,address,size,flags*

**segment_name** can be the name of an existing segment or a new name,
in which case a new entry will be created in the segment table. The
segment name must be enclosed in double quotes. It is the only required
parameter, all other values can be left omitted or left blank (i.e.
using multiple commas) to preserve existing values, or use default ones.

**segment_number** provides an alternative syntax to the above, in which
you provide the number of the segment, as it appears in the segment
table. By definition, the data segment's number is 0, and the blank
(entry) segment is number 1. You cannot create new segments with this
method, as the name has to be entered one way or another.

**page** is the page you want the segment to be loaded in. If you change
this, you should probably also change the address.

**address** is the address where the segment will be loaded at. Check
segment size to make sure there is room enough in the page.

**size** is the size of the segment. Normally, there is no reason to
modify this.

**flags** are the segment flags, used to alter the dispatching strategy.
Any change to these will only take effect the next time the dispatcher
routine is called (e.g. with the `DISPATCH `command). See my [USB DSR
page](usb_dsr.htm#Segment%20flags) for legal flag values.

After execution, \_PTR points at the segment's entry in the segment
table, or contains zero if the segment name was not found.

Example:

    ASSIGN SEG1,7,4F00         ; assign segment "SEG1" to adress >4F00 in page 7
    ASSING SEG2, , , ,>0001    ; modify segment "SEG2" flags

####  DISPATCH command

This command causes the loader to scan the EEPROM and see if it can fit
all current segments into it. Nothing is actually loaded, but the
segment table is updated accordingly. Because a DISPATCH is
automatically executed after each LOAD, there aren't many reasons why
you would want to trigger it yourself. But there may be cases when you
want to execute a modified version of the command:

DISPATCH MORE

Whereas the regular dispatch resets all page assignments and restarts
from scratch, the MORE mofidier (only "M" is necessary) causes it to
retain any valid page assignments. Only segments with an address of zero
will be assigned to a new location.

After execution, \_ITEMS contains the number of segments that have been
reassigned.
DISPATCH UNIT,n

This causes the dispatcher to place segments at the same page and
address they were in a previously loaded unit. Since segment names are
not saved in units, all segments should appear in the same order now
than when the unit was first loaded. In addition, only segments with the
same size as before will be assigned their previous locations. After
execution, \_ITEMS contains the number of segments that were
successfully dispatched.

This feature is useful when you are writing your own DSRs and made
changes to only one segment: instead of erasing the EEPROM and reloading
the whole file, you can just reload the segment you modified. For
instance, let's say the previous version you loaded was unit \#6, and
you subsequently modified segment MYSEG. You could type (or run a
script) something like:

    LOAD DSK1.MYFILE
    DISPATCH UNIT,6
    ASSIGN MYSEG,0,0       ; this clears assignments for segment MYSEG
    DISPATCH MORE          ; reassigns MYSEG to a new location
    BURNW MYSEG            ; burn new version of MYSEG

Notes: The `ASSIGN `statement is not necessary if the modifications you
made resulted in changing the size of MYSEG, since `DISPATCH UNIT` will
only consider segments whose size in the file is the same as it was in
the unit.

For your program to work properly, the changes you made should not
modify the entry point of any routine that is called from another
segment: since these will not be reloaded, they will keep calling the
old addresses.

####  PG4SEG command

This command triggers a verification mechanism for the PG4SEG label.
When included in your program as a REF, every occurence of the PG4SEG
label will be replaced with the page number for the segment in which the
previous address is located. For instance:

           BLWP @SGBL
           DATA MYSUB,PG4SEG

will replace PG4SEG with the number of the page in which the segment
containing MYSUB is to be loaded.

The command allows for two types of verification:

`PG4SEG 1 `ensures that every PG4SEG is preceded with an address in
another segment
`PG4SEG 2 `ensures that every address refering to another segment is
followed with a PG4SEG
`PG4SEG 3` combines both of the above
`PG4SEG 0` turns verification off.

Type 1 violations are announced individually. The symbol \_PTR will
contain the address of the "orphan" PG4SEG.
Type 2 violations are announced only once. The symbol \_PTR will contain
the address of the first offending word, i.e. one that contained a label
defined in a different segment, but was not followed with a PG4SEG
label.

###  Script-related commands

The following commands only make sense when included into a script file.
A script file is a DV80 text file containing a list of commands, one per
line. To launch a script file, just type its name on the command line:
it will be loaded into the VDP memory and execution will begin
immediately. Alternatively, you can use the LOAD command.

A script can call another script file, but there is no mechanism to
automatically return to the caller. In fact, the caller is wiped out of
memory when the second script is loaded. Of course, the second script
may contain a LOAD command returning to the first file, but script
execution will always begin at the top of the file.

Comments can be added at the end of each line, after a ; semicolon.
Alternatively, you can have a whole line commented out, if it begins
with a semicolon. You can also include empty lines, to make the script
easier to read.

[ECHO
](#Echo)[GETKEY
](#GETKEY)[GETVALUE
](#GetValue)[IF
](#If)[ELSE
](#ELSE)[ENDIF
](#ENDIF)[ELSEIF
](#ELSEIF)[ANDIF
](#ANDIF)[ORIF
](#ORIF)[GOTO
](#GOTO)[STOP
](#STOP)[COMMAND](#COMMAND)

####  ECHO command

There are two ways to use this command: the first one is to use it with
an YES/NO switch (only the first letter is scanned). This determines
whether the contents of the script file will be displayed on screen
while it is processed. The default is NO.

The second way is to follow ECHO with one or more quoted strings to be
displayed onscreen. The size of each string should not exceed 28
characters. Optionally, one or more / slash caracters can be added
outside of the quote marks, each will cause the display to scroll up one
line.

You can also use echo to display the value of a given symbol from the
symbol table: just use a @ sign followed with the symbol name (outside
the quote marks). The symbol value is displayed as an hexadecimal word,
and the segment number is not displayed.

Examples:

    ECHO YES
    ECHO "Press any key to proceed"
    ECHO "Press:" // " 1 to continue" / " 2 to end"
    ECHO ////////// ////////// ///   ; this clears the screen
    ECHO @_PTR  ; displays the value of this symbol

####  GETKEY command

This command scans the keyboard and waits for the user to press a key.
When this happens, the ascii code for the key is placed into a symbol
called \_KEY.

N.B. It's generally a good idea to precede GETKEY with an ECHO statement
prompting the user for a key...

####  GETVALUE command

This command inputs a numeric value from the user and places it in the
symbol table, in a symbol called \_VALUE. The symbol called \_PTR will
contain a pointer to the string actually typed in by the user.

N.B. It's generally a good idea to precede GETVALUE with an ECHO
statement prompting the user for it.

####  IF command

This command lets you process sections of the script file only if some
condition is met. For instance, if the user presses a key, or if a given
export was found, etc.

If the condition is true, the script continues with the next line. If it
is not true, the script will be read but not executed until an ELSE,
ELSEIF, ORIF or ENDIF is met. You can nest as many IFs as you like, as
long as you can keep track of them.

The syntax is:

    IF name,operator,value

**name** is the name of a symbol. If it does not exist, the command is
ignored (i.e. true). With the IN (or NIN) operator, *name* can also be a
segment, a DSR, a CALL, or an export. The name doesn't need to be
enclosed in double quotes, although these are accepted.

**operator** is the type of comparison to be performed. Legal operators
are:

EQUAL is equal to
GREATER is arithmetically greater than (signed)
SMALLER is arithmetically smaller than (signed)
HIGHER is logically higher than (unsigned)
LOWER is logically lower than (unsigned)
IN is among

All operators can be abbreviated upto one letter, as only the first
letter is checked. In addition, each operator can be preceded with the
letter N to invert the comparison:. NEQ means "not equal, NG means "not
greater than" (i.e. smaller or equal), etc.

**value** is the value the symbol should have for the IF to be taken. As
usual with numeric values, you could specify another symbol preceeded
with a @ sign: `IF MYSYM,EQU,@OTHER`

The exception is the with IN (or NIN) operator: the numeric value is
replaced with the category to search for the name. Valid categories are
(each can be abbreviated downto one letter):

LABEL searches the symbol table for DEF or EQU symbols.
REF searches the symbol table for solved REF entries.
UNRESOLVED searches it for unresolved REFs.
SEGMENT searches the segment table for a segment with this name.
DSR searches the DSRs in page 0.
CALL searches the CALLs in page 0.
EXPORTS searches the export list in the EEPROM.

After a succesful IN, \_PAGE will contain the number of the page the
name was found in (zero for symbols) and \_PTR will point the the name's
entry in this page. If the name was not found, \_PTR and \_PAGE will be
zero.

Exemples:

    IF MYDSR,IN,DSR    ;true if a DSR called MYDSR exists in the EEPROM
    IF THIS,NIN,EXP    ;true if no export called THIS exists in the EEPROM
    IF MYLAB,EQU,1234  ;true if symbol MYLAB has a value of >1234

####   ELSE command

This, as you might expect, serves to reverse the conditional execution
of an IF. If the IF was taken, everything included between the
corresponding ELSE and the corresponding ENDIF will be read but not
executed. If the IF was not true, and thus not executed, the occurence
of an ELSE resumes execution of the script.

Note that including an ELSE without a matching IF is allowed. Since
execution is "on" by default, a lone ELSE will stop execution until a
matching ENDIF is met. The same is true for ELSEIF, ANDIF and ORIF.

####  ENDIF command

This ends a conditional section, whether defined by an IF or an ELSE.
From now on, execution of the script resumes normally (unless the
IF-ENDIF was nested inside another, of course).

####  ELSEIF command

This commands combines and ELSE and an IF: if execution was on, it will
now stop. If execution was stopped, the expression will be evaluated
and, if it's true, execution will resume. The syntax is identical to a
regular IF command.

One use of ELSIF is to separate multiple, mutually exclusive, clauses:

    IF A,EQ,0
    ; do something if A=0
    ELSEIF A,GT,0
    ; do something else if A > 0
    ENDIF   ;do nothing if A < 0

####  ANDIF command

This commands lets you combine several conditional expressions. Its
syntax is identical to that of a regular IF. If execution was stopped,
it remains stopped. If execution was on, the new statement is evaluated
and, if false, execution will stop. In any case, only one ENDIF is
required to end conditional execution for both the IF and any subsequent
ANDIF.

    IF A,GT,5
    ANDIF A,LT,9
    ; executed only if A equals 6, 7, or 8.
    ENDIF

####  ORIF command

This command lets you combine several conditional expressions. Its
syntax is identical to that of a regular IF. If execution was on, it
remains on. If execution was stopped, the new statement is evaluated
and, if true, execution will resume.

    IF A,EQ,5
    ORIF A,EQ,8
    ; executed if A equals 5 or 8
    ENDIF

N.B. You can also use ORIF instead of ELSEIF to separate mutually
exclusive clauses.

####  GOTO command

This command is used to jump to another location within the script file.
It is followed with the name of a label that must have been defined with
a prior EQUATE \$ statement (i.e. you can't jump forward: use an IF
instead).

    EQUATE HERE,$         ; HERE points to this line
    ...                   ; some code
    GOTO HERE             ; loops back to HERE

Be aware that a RESET in a script wipes out the symbol table and thus
forgets about any previously defined label. In such a case, consider
using RESET SEGMENTS instead.

####  STOP command

This command aborts the execution of a script and returns to command
mode. It is not necessary to include it at the end of the script,
although it won't hurt.

####  COMMAND command

This command lets the user type in a command and executes it. Then it
resumes script execution.

###  Predefined symbols

The loader contains a number of predefined symbols, which fall in two
categories. The first consists in routines defined within the default
stub. These can be called from your DSR programs with a simple REF
statment. Their usage is detailed in [another page](usb_dsr.md).

**SGB** Branches to a routine in another segment.
**SGBL** Branch and link to a routine in another segment.
**SGRT** Return from such a routine.
**SGBLWP** Branch and load workspace pointer to a routine in another
segment.
**SGRTWP** Return from such a routine.
**ATR11** Get data word following a call to SGBL.
**ATR14** Get data word following a call to SGBLWP.
**SGBLX** Branch to a routine in another segment, save return point and
page on stack.
**SGRTX** Return from a routine called in this way.
**ATR11X** Get data word following a call to SGBLX.
**GETDAT** Get data from another segment.
**RAMPG** Select a different SRAM page.

There are also three variables, whose content varies during loading:

**MYSLOT** Address of a reserved 4-byte slot in SRAM memory.
**CURPG** Current EEPROM page, where the current segment resides.
**PG4SEG** Page for the segment which contains the label used in the
previous word.

The second category consists in variables internal to the loader. They
all begin with an underscore character, which is illegal in assembly, so
that there is no chance they will interfere with a label defined in your
program. But you can view them with the SYMTAB or ECHO @ commands,
modify them with EQUATE, and use them wherever a numeric value is
required.

**\_VALUE** Result of the GETVALUE command.
**\_KEY** Result of the GETKEY command.
**\_PAGE** Modified by many commands, page number.
**\_PTR** Modified by many command, pointer to a memory location.
**\_ITEMS** Modified by many commands, number of items listed.
**\_CRU** Current CRU for the USB-SM card.
**\_FSTPG** First EEPROM page where to load DSRs (normally 3).
**\_LSTPG** Last page in the EEPROM.
**\_PAGE0** Page where to load entry points (normally 0).
**\_EXPG** First page for the list of exports (normally 1).
**\_UNIPG** First page for units removal info (normally 2).
**\_MAXSZ** Maximum size for a segment.

###  Command summary



------------------------------------------------------------------------

The USBHOST demo DSR

This demo DSR is meant to let you access USB devices (via the USB host
controller) from Basic or Extended Basic.

To load this demo DSR, first launch the EEPROM manager as described
above. Then type in DSK1.USBHOST. This will call a script that loads two
DSR files: STDMEM/O (the standard memory management routines) and
USBHOST/O (the demo DSR itself).

Once done, enter Basic or Extended Basic. Everything is handled through
a pseudo-file called USBHOST.

#### Opening the pseudo-file

First, you should specify which device you want to talk to, by including
it in the filename. All devices answer to number zero, so you can
detemine how many are connected, and assign a number to each. Once this
is done, you can access each device as an independent file, by using the
number you assigned to it.

    100 OPEN #1:"USBHOST.nnn",INTERNAL,FIXED 255,UPDATE,RELATIVE

Where **nnn** is the device number, from 0 to 127. You could actually
open the file with any record length, but 255 is more convenient to
transfer large chunks of data.

The pseudo-file consists of 5 records, each corresponding to a different
operation:

REC 0 sends "SETUP" packets (i.e. commands) to the device.
REC 1 sends "OUT" data packets to the device.
REC 2 sends "IN" packets, i.e. fetches data from the device.
REC 3 controls the two downstream ports at the back of the card.
REC 4 controls interrupts.

The first three records are used to talk to USB devices. Each one must
be first written to, then read. No other access to USBHOST should occur
in between the write and the read operations.

####  Writing to a device

Record 0 and 1 are used in the same way, the only different being the
packet ID: SETUP vs OUT.

    110 PRINT #1, REC 0:A$,endpoint,maxbytes,speed,toggle1
    120 INPUT #1, REC 0:err,actbytes,toggle2

**A\$** is the data you want to send to the device. It may be an empty
string, in which case an empty packet will be sent (usefull for
acknolewdging an "IN" packet for instance).

***endpoint*** is the endpoint number, from 0 to 15. Each device is
guaranteed to have endpoint 0. For the other endpoints you must first
check the device configuration (by sending the
GetConfigurationDescriptor command via a SETUP packet). If this
parameter is ommited or negative, the endpoint most recently used (with
any device) will be used again. At power-up time, the default is
endpoint zero.

***maxbytes*** is the maximum number of bytes per packet. If this is
less than the number of characters in A\$, the host controller will
split the operation into multiple packets, alternating the toggle bit
between 0 and 1 for each of them. If this parameter is ommited or
negative, the value most recently used (with any device) will be reused.
At power-up time, the default is 8.

***speed*** is the USB transfer speed. Odd numbers select half-speed,
even numbers full-speed. If this parameter is ommited or negative, the
last speed used with this device will be reused. At power-up time, the
default is full-speed. With device 0, the correct speed is automatically
determined when opening the file, and used as a default.

***toggle*** is the initial value of the toggle bit used when sending
data packets: 0 for DATA0, 1 for DATA1 packet ID. If this parameter is
ommited or negative, the most recent value used by this device will be
inverted. At power-up time, the default is 1.

***actbytes*** is the actual number of bytes transfered. If it's lower
than the size of A\$, an error probably occured.

***toggle2*** is the value of the toggle bit for the last packet that
was sent.

***err*** is the error code returned by the controller:

     0: No error
     1: CRC error in last data packet from endpoint
     2: Bit stuffing violation in last data packet from endpoint
     3: DATA0/DATA1 toggle mismatch
     4: Endpoint returned a "STALL" PID
     5: Device not responding
     6: PID check bits mismatch
     7: Unexpected PID (illegal value, or legal but at the wrong time)
     8: Data overrun, i.e. the endpoint returned more than "max bytes" in a packet, or more than "total bytes".
     9: Data underrun. i.e. the endpoint returned less than "total bytes", yet less than "max bytes" (i.e. no more coming).
    12: Buffer overrun. During an IN, the host controller received data faster than it could pass them to the system.
    13: Buffer underrun. During an OUT, the host controller could not get data from the system fast enough to send them.

####  Reading from a device

USB being an host-centric system, all transmissions must be initiated by
the host. Thus, to read data from a device, you must first query it.
This is done via record 2:

    130 PRINT #1, REC 2: totbytes,endpoint,maxbytes,speed,toggle1
    140 INPUT #1, REC 2: B$,err,actbytes,toggle2

***totbytes*** is the total number of bytes that you are expecting to
receive from the device. If this parameter is ommited or negative, the
value most recently used (with any device) will be reused.

**B\$** is a string that will contain the data sent by the device.

All other parameters have been described above.

####  Controlling the ports

Record 3 is used to control the card ports. It can be read or written to
independently. Reading returns the port status, writing controls the
port. Let's examine reading first:

    200 INPUT #1, REC 3: port1, port2, chg1, chg2, hub

***port1*** is the status of the bottom port.

***port2*** is the status of the top port.

The status breaks down to the following bits:

    512: low speed device connected
    256: port power is on
     16: resetting port
      8: overcurrent detected
      4: port is suspended
      2: port is enabled
      1: device connected

***chg1*** indicates changes to port1 since last time it was read.

***chg2*** does the same for port2.

     16: reset completed
      8: overcurrent bit has changed
      4: suspended bit has changed
      2: enabled bit has changed
      1: connected bt has changed

***hub*** is the global status of the hub controller and breaks down to
only two bits:

    32768: remote wakeup occured
        2: overcurrent detected
AS mentionned above, writing to record 3 controls the ports:

    220 PRINT #1, REC 3:set1,set2

***set1*** sets the bottom port.

***set2*** sets the top port.

Writing a zero has no effect, which is convenient when you need to set
port 2 without affecting port 1. The set word is a combination of the
following values:

    512: turn power off
    256: turn power on
     16: reset the port
      8: resume suspended port
      4: suspend port
      2: enable port
      1: disable port

####  Using interrupts

Record 4 is used to control the interruption mechanism: upon certain
condition, the DSRs can interrupt your program and branch automatically
to a predefined line number. This way, you don't have to constantly
monitor the USB controller to know if a device was plugged in, if data
came in, etc. Note that the mechanism suffers from a major flaw: Basic
statements that do not continue with the next line cannot be interrupted
(e.g. GOTO, NEXT, IF, etc). But, hey, this is just a demo DSR...

To setup the interrupt mechanism, write to record 4:

    300 PRINT #1, REC 4:mask,line

***mask*** specifies which event you want to trigger an interrupt. It is
a combination of the following values:

    16384: clock is ready (160-1000 usec after a wakeup).
     8192: host controller suspended.
     1024: transfer completed (to/from CPU).
      512: ATL should be read.
      256: ITL should be read.
       64: change in port status or hub status.
       32: frame number overflow.
       16: unrecoverable error.
        8: device resuming signal.
        4: start-of-frame occured (happens every millisecond!).
        1: scheduling overrun.

***line*** is a line number where the interrupt should branch. It must
be a REM line, immediately followed with a statement accessing record 4.
This second line number should be exactly 10 higher than the REM line.
NB: if you resequence you program, you will need to update the value
you're setting up.

If *mask* is negative, special actions are taken depending on the value
of *line*:

If *line* is negative or missing, interrupts are disabled. Basic
execution continues normally.

If *line* is zero, interrupts are enabled and Basic execution continues
at the point where is was interrupted.

If *line* is positive, interrupts are enabled and Basic execution
continues at the line immediately following *line*.

To check the current interrupt status, just read record 4:

    1000 INPUT #1, REC 4:ints,last

***ints*** returns the interrupt flags, i.e. lets you know which event
has triggered an interrupt. It is a composite of the bit values
described for *mask*.

***last*** is the number of the last line executed before the interrupt
caused the program to jump to the REM line. This may be usefull for you
to know what your program was doing before it got interrupted.

Reading record 4 will also clear the interrupt condition, allowing your
program to continue normally.
Example:

    100 OPEN #1:"USBHOST.1",INTERNAL,FIXED 255,UPDATE,RELATIVE   ! open device #1
    110 PRINT #1, REC 4:64,790          ! interrupt when port status changes
    120 ...                             ! more code

    790 REM   Interrupt service routine ! This must be a REM line
    800 PRINT #1, REC 4:-1              ! disable interrupts
    810 INPUT #1, REC 4:X               ! check status, clear interrupt condition
    820 IF X<>64 THEN ...               ! if not our interrupt, do something
    830 ...                             ! if it is do something else (e.g. check port status)
    890 PRINT #1, REC 4:-1,0            ! enable interrupts, return to interrupted point
For more examples of how to use USBHOST, check out the following Basic
programs:

GETDESC demonstrates how to get a device descriptor.
PORTS plays with port status.
INTS demonstrates interrupts.
DIRECT demonstrates direct access to the host controller.

Revision 1. 8/15/04 Ok to release.
Revision 2. 1/2/04 Several changes. Added demo DSR.

Revision 3. 1/31/05 Added download instructions.
Revision 4. 2/6/05 Minor cosmetic changes.
[Previous page](usb_dsr.md) (Writing DSRs)

[Back to the TI-99/4A Tech Pages](titechpages.md)
