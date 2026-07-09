# The Editor/Assembler module

The Editor/Assembler module is a pure GROM cartridge. It contains only
one GROM, with addresses \>6000-7800. It answers to any GROM base
(\>9800 to \>983C).

Strangely enough, the GROM does not contain an editor, nor an assembler:
these are assembly language programs provided on a companion disk. All
the cartridge does is to display nice menus that let you load these
programs (or others of your choice). On the plus side, the cartridge
contains a linking-loader that can process the tagged-object files
produced by the assembler and several subroutines to be called from
TI-Basic.

If you wish, you can view a commented disassembly of that GROM in three
parts: [main program](ea1.txt), [subprograms](ea2.txt), [assembly
routines](ea3.txt).
**Program  **
[Initialisation
](#Program%20init)[Main menu
](#Main%20menu)[Editor menu
](#editor%20menu)[Assembler menu
](#assembler%20menu)[Load-and-run menu
](#load-and-run%20menu)[Run menu
](#run%20menu)[Run-program-file menu](#program%20menu)

**Subprograms  **
[INIT
](#subprograms)[LOAD
](#load)[LINK
](#LINK)[POKEV
](#pokev)[PEEK
](#peek)[PEEKV
](#peekv)[CHARPAT](#charpat)

**Programming tricks  **
[Forced loading
](#forcing%20loading)[Preventing "undefined" symbol errors
](#no%20undefined)[Using the illegal tags
](#illegal%20tags)[Biased loading
](#loading%20bias)[Extra assembler opcodes](#Extra%20opcodes)

**File formats  **
[EA5 (program file)
](#EA5)[Tagged object code (DF80 file)](#tagged-object)

##  Program

The GROM contains only one GPL program: EDITOR/ASSEMBLER, whose start
point is \>6025 in GROM memory.

###  Initialisation

When entered, the program loads the built-in character sets from the
console GROM:

small upper-case chars 32-95 at VDP address \>0900-0AFF,

lower-case characters 96-127 at VPD address \>0B00-0BF7,

empty chars 0-31 at VDP address \>0800-08FF, except for:

copyright character (10) at address \>0850-0857,

editor cursor (a solid 8x3 pixels vertical bar) at address \>08F0-08F7
(i.e. char 30),

module cursor (an empty 6x6 square) at address \>08F8-08FF (i.e. char
31).

For some reason, provision is made to load a different set of
characters, possibly corresponding to another console version: the
distinction is made by scanning the keyboard with keyboard type 5. If
the type is not changed to 0 by the scanning routine, characters \>32-95
will be loaded at addresses \>0A00-0BFF in VDP memory, then characters
64-95 are modified by adding a small 2-pixel dash in their upper left
corner. I have no idea what this is meant for, the TI-99/4 may be?

The VDP registers are then set with the following values:

R1=E0 Standard mode
R2=00 Screen image at \>0000
R3=0E Color table at \>0380
R4=01 Char pattern table at \>0800
R5=06 Sprite attribute table at \>0300
R6=00 Sprite pattern table at \>0000 (not the same as char pats!)
R7=F5 White on blue

###  Main menu

data-bgcolor="#1822cd">
 * EDITOR/ASSEMBLER *
                                PRESS:
  1 TO EDIT
  2    ASSEMBLE
  3    LOAD AND RUN
  4    RUN
  5    RUN PROGRAM FILE

  c)1981  TEXAS INSTRUMENTS

Option 1 enters the editor menu.
Option 2 calls the assembler.
Option 3 is used to load an assembly file in tagged-object code, it
enters option 4 when done.
Option 4 is used to run a program loaded in memory with option 3.
Option 5 is used to load and run a memory-image file, in "program"
format.
Fctn-9 resets the TI-99/4A.

###   The editor menu

data-bgcolor="#1822cd">
                                PRESS:
   1 TO LOAD
   2    EDIT
   3    SAVE
   4    PRINT
   5    PURGE

     

Option 1 causes the editor to load a DV80 or a DF80 file in memory.
Option 2 enters the editor with the file currently in memory (if any).
Option 3 causes the editor to save the file currently in memory.
Option 4 prints a file.
Option 5 purges the file currently in memory (so that option 2 can be
used to edit an empty document).
Fctn-9 returns to the main menu.

Options 1 to 3 first check whether the editor is already in memory. This
is done by looking for a flag value of \>55AA in word \>2000. If the
editor isn't in memory, it is loaded from a "program" file called
"EDIT1". This file must be in the standard "Option 5" format, which
means that loading could continue with EDIT2 (it won't). The program
then automatically performs the equivalent of option 5 (i.e. clears 3
words at \>FFD8).

Option 1 prompts the user for a file name and prepares a PAB for that
file in VDP memory at address \>1000 (pointed at by word \>831C). The
data buffer for that PAB will be located at \>1080. The program then
calls subroutine \>0010 in GROM memory to attempt opening the file as a
Div/Fix 80 file (the format of object files). If it fails, the program
changes the file type to Div/Var 80 (the format of source files) and
tries again. If it fails again an I/O error is announced, otherwise the
editor is entered via an XML \>21 (i.e. vector in \>2002).

Option 2 enters the editor via an XML \>23 (i.e. vector in \>2006). Upon
return, VDP register 1 is put in standard mode again, the GPL Cnd bit in
\>837C in checked. If it is set, the program displays an error message
whose number is taken from byte \>8322, otherwise it just returns to the
editor menu.

Option 3 displays "`VAR 80 FORMAT(Y/N)`?" to know whether the file
should be saved as a source file in DV80 format, or as an object file in
DF80 format. Then it prompts the user for a filename and creates the
PAB, just like option 1. Finally it calls the editor via an XML \>22
(i.e. vector in \>2004).

Option 4 prompts the user for a filename just as options 1 and 3. Then
it asks for a printer name. If the name is "TP" (Texas Instruments
thermal printer) the default record length is set as 32 chars, but the
printer is given a chance to modify this value. For all other printers,
the record length is set as 80 chars. The program creates a PAB for the
printer in at address \>1100 in VDP memory. Records are then read from
the file and written to the printer until an I/O error \#5 occurs (i.e.
"past eof"). The error is not announced though: instead both PABs are
closed and the programs returned to the editor menu.

A special processing occurs for the thermal printer: all trailing spaces
are removed, and char 12 (form feed) is replaced with four spaces. Then
the 80-byte long file records are split in chunks corresponding to the
record length of the printer.

Option 5 displays "`ARE YOU SURE(Y/N)?`" If the user presses "Y" the
routine clears three words in the high-mmemory expansion: \>FFD8-FFDD.

###   The assembler menu

data-bgcolor="#1822cd">
                                 SOURCE FILE NAME?
  user input
  OBJECT FILE NAME?
  user input
  LIST FILE NAME?
  optional user input
  OPTIONS?
  optional user input
     

The module first checks whether the assembler is already in memory. This
is signaled by a flag value of \>AA55 in word \>2000. If this is not the
case, the programs displays "`LOAD ASSEMBLER(Y/N)?`", the user can
answer "N" to avoid overwritting a program in memory, in case the
assembler menu was entered by mistake (in the Editor menu, you must
press two keys before anything is loaded, therefore no confirmation is
required). If the user elects to load the assembler, the module loads it
as a "program" file in EA5 format, named ASSM1, loads it and proceeds
with a file named ASSM2. It then uses the GPL routine \>0010 to call
subroutine \>16 in the disk controller (this is the assembly equivalent
of a CALL FILES) and sets the number of available files to four.

At this point, the user is successively prompted for:

- A source file, whose PAB will be at \>1000 in VDP memory, with a
  buffer area at \>1080. This must be an existing Dis/Var 80 file.
- An object file, whose PAB will be at \>1100 in VDP memory, with a
  buffer area at \>1180. This must be a Dis/Fix 80 file, but it may not
  exist yet.
- An optional list file. If a name is entered, the PAB will be at \>1200
  in VDP memory, with a buffer area at \>1280. The file must be of type
  Dis/Var 80, but it may not exist yet (or it may be a printer name).
- An optional string of upto 15 characters that will be passed to the
  assembler at address \>20D2 in cpu memory. Typically, this string
  contains assembler options: R= use Rx notation for registers, L = use
  a list file, C = compressed the output, etc.

Finally, the sceen is cleared and the assembler is called via an XML
\>21 (i.e. vector in \>2002). Upon return, the Cnd bit in the GPL status
byte is checked for a possible error (whose number will be in \>8322),
otherwise the files are closed, and the message
"`PRESS ENTER TO CONTINUE`" is displayed at the bottom of the screen.
This allows the user to read any messages issued by the assembler
(generally syntax errors...). Once the user presses \ the
program returns to the main menu. However, the assembler is still in
memory and won't be reloaded if the user enters the assembler menu
again.

###   The load-and-run menu

data-bgcolor="#1822cd">
                                 FILE NAME?
  user input (multiple)
  PROGRAM NAME?
  user input
  

     

This option prompts the user for a file name and creates a PAB for it at
address \>1000 in VDP memory. It then checks whether a memory expansion
is installed (by writing test values at \>2000 and reading them back)
and issues an error message if this is not the case. Otherwise, the
program downloads assembly language routines stored in GROM at
\>7000-7730 into the low memory expansion at addresses \>2000-2007,
\>2022-2676 and \>3F38-3FFF. One of the routines is the linking-loader
itself. The area above \>3F38 is the beginning of the symbol table used
by the loader (growing downwards) and contains pre-defined values.

The loader is then called via an XML \>22 (i.e. vector in \>2004, just
loaded above). The loader can return with an error by setting the Cnd
bit in the GPL status byte and placing the error code in byte \>8322.
The module will then issue an error message and (in most cases) re-enter
the load-and-run menu.

If all went OK, the user is prompted for another filename, just as
above. Note that the program sets a flag bit in byte \>8349 so that the
assembly routines won't be loaded again. The loader is then called again
and the whole process repeats until the user enters an empty filename.

At this point, the module looks for a flag value of \>A55A in word
\>2000 (indicating that the assembly routines were loaded). If the
correct value is not found the assembly routines will be loaded as
above. In any case, it the branches to the "RUN" menu.

###   The run menu

data-bgcolor="#1822cd">
                               
  PROGRAM NAME?
  user input


 

This menu can either be entered from the main menu, which displays the "
`* RUN * `" title, or from the "load-and-run" menu, after the user
entered an empty filename.

The user is prompted for a 6-character program name. This must
correspond to an assembly label, i.e. contain only upper-case letters
and digits, the first character cannot be a digit. This label should be
present in the symbol table, either because it's part of the pre-defined
table or because it was placed here by the loaded (as a result of an
assembly-language DEF statement).

The module checks for a flag value of \>A55A in word \>2000, indicating
that the standard assembly language routines were dumped from the GROM.
If the value is not found, a "`PROGRAM NOT FOUND`" error is issued.

Then the module sets up the VDP for the program to be executed: the
color table is filled with \>13 bytes (black on light green), the screen
is erased, and VDP register 7 is set as \>F3 (white on light green).
Finally, the linker is entered via an XML \>21 (i.e. vector in \>2002).
Upon return, the Cnd bit in the GPL status byte will cause an error
message, if it's set. The error code is taken from byte \>8322, that was
cleared before calling the linker. Otherwise, the VDP setup of the
module is restored, and the message ""`PRESS ENTER TO CONTINUE`" is
displayed at the bottom of the screen.

When the user presses \ the program returns to the main menu.
Note however that nothing is erased in memory, so the user can select
option 4 again and run another program, or the same one. To run the same
program again, the user can just enter an empty program name.

###   The program menu

data-bgcolor="#1822cd">
                               

  PROGRAM NAME?
  user input (optional)
  `
  ONE MOMENT PLEASE...

The user is prompted for an optional file name. This must be an existing
"program" file containing a memory dump with a six-byte header (a
[format](#EA5) sometimes known as EA5, for Editor/Assembler option 5).
If the user enters an empty string, the filename "UTIL1" will be used. A
PAB for the file is prepared at VDP address \>1000, with a data buffer
area at \>1380 and a maximum lenght of \>2100 bytes.

If the filename begins with "CS" the screen will be cleared to allow for
proper display of the cassette management prompts. Then the program then
displays "`ONE MOMENT PLEASE...`" at the bottom of the screen and calls
the DSR that will load the file in VDP memory. It then transfers the
file in CPU memory, according to the instructions of the 6-byte header.
If the header calls for additional files, the last character of the
filename in incremented by 1 (i.e. UTIL1 becomes UTIL2, etc), and the
process starts again.

Otherwise, the module sets up the VDP for the program to be executed:
the color table is filled with \>13 bytes (black on light green), the
screen is erased, and VDP register 7 is set as \>F3 (white on light
green). Finally, the program is entered at the address where the first
file was loaded, using an XML \>F0 (i.e. vector in \>8300).

Upon return (note that many third-party programs do not return, they
just reset the TI-99/4A), the VDP setup of the module is restored, and
the Cnd bit is tested in the GPL status byte \>837C. If it is set, an
error code taken from byte \>8322 is displayed in hexadecimal. In any
case, the module displays "PRESS ENTER TO CONTINUE" at the bottom of the
screen, and waits for the user to react. It then returns to the main
menu.

Note that it is not possible to re-enter the user program without
reloading it.

####  Creating EA5 memory-image files

No provision is made in the cartridge to produce EA5 files. However, the
accompanying disk comes with a small utility called SAVE. This utility
REFerences three labels that should be DEFined by the program to save:
SFIRST is the first address to save, SLAST is the last address to save,
and SLOAD is where the program should be loaded by option 5 (generally,
SLOAD is identical to SFIRST, but this is not an absolute requirement).
Load your program first with option 3, then load SAVE: it will autostart
upon loading and automatically save the program on a "program" file
called DSK1.UTIL1

You can do the same (and more) with my MISS utility,
[downloadable](download.md) from this website. Or you could use R.A.
Green's RAG linker produce a memory-image EA5 file directly from the
tagged-object DF80 files.

##   Subprograms

### INIT

All this subprogram does is to load the assembly language subroutines
stored in GROM at \>7000-7730 into the low memory expansion at addresses
\>2000-2007, \>2022-2676 and \>3F38-3FFF. It therefore empties the
symbol table and re-initializes the loading pointers.

###  LOAD

This subprogram can actually perform two distincts functions: place
numeric values at a given address in cpu memory, or call the loader and
load a tagged-object file into memory. Which function is selected
depends on the type of parameter passed to the subprogram: a (non-empty)
string constant or string variable calls the loader, a number or a
numeric variable patches the memory.

When a filename is passed as an argument, e.g. CALL LOAD("DSK1.MYFILE"),
the subprogram loads the standard assembly language routines then calls
the loader with XML \>22. Upon return, if the Cnd bit of the status byte
is set, an error will be returned. Otherwise, LOAD resumes parsing the
parameters. This means you can load several files, or mix file loading
and memory patching (however, once you started memory patching, you
cannot load any more files).

To load values into memory, you must first specify the destination
address as a number from -32768 to +32767, where nagetive numbers
correspond to addresses in the range \>8000-FFFF. LOAD calls XML \>12 to
convert the real number into an integer. The following parameters must
contain the bytes to place in memory, therefore the useful range is
0-255. Note that you can specify any value from -32768 to 32767, but
only the least significant byte of the integer will be loaded.

If you want to specify a new address in the same CALL LOAD statement,
you can use an empty string constant (or variable) as a separator. The
string must be empty so that it is not mistaken for a filename:
non-empty strings issue a "bad argument" error.

#### Example:

CALL LOAD("DSK1.MYFILE","DSK2.HISFILE",-24576,65,66,"",-20480,0,218,)

Loads a tagged-object code file called MYFILE into memory,
loads the file HISFILE into memory,
places bytes 65 and 66 (i.e. "AB") at location \>A000,
and places bytes 0 and 218 at location \>B000.

###  POKEV

Works exactly like LOAD, except that the target is the VDP memory. The
address should therefore be in the range 0-3FFF (i.e. 16383). Of course,
as programs can't run in VDP memory, it is not allowed to pass a
filename within a CALL POKEV.

###  PEEK

This subprogram performs the opposite of LOAD: it fetches bytes from VDP
memory and transfers them in numeric variables.

Just like LOAD, you must specify an address in the range -32768 to
+32768 either as a number or as a numeric variable. Then you can enter
as many numeric variables as you like: each will receive one byte
corresponding to the content of the cpu memory, starting at the
specified address. Values will be in the range 0-255.

You can use an empty string as a separator to enter a new address, but
non-empty strings will cause an error.

#### Example:

CALL PEEK (12,CLOCK,"",72,A,B,C,D)
XOP2WR=(256\*A)+B
XOP2PC=(256\*C)+D

Gets the content of byte \>000C into the variable CLOCK (this is the
console clock frequency: 48 for 3 MHz or 40 for 2 MHz).

Then gets the vectors for extended operation 2 (which does not exist in
all consoles). The next two statements combine the bytes into words:
XOP2WR should be 33696 (i.e.\>83A0) and XOP2PC should be 33536 (\>8300).

###  PEEKV

Does the same as PEEK, but gets values from the VDP memory. Addresses
should therefore be in the range 0-16383 (i.e \>3FFF).

###  LINK

This subprogram is used to begin execution of a program loaded with CALL
LOAD. The first argument should be a quoted string, or a string variable
containing the program name (i.e. an assembly DEFined label).

Optionally, you can specify upto 16 additional parameters that will be
passed to the executed programs. LINK fetches each parameter from the
Basic symbol table, concatenates & strings if necessary, and places a
numeric value or a string pointer on the value stack in VDP memory.
These values can be retrieved be the called program via XML \>18.

In addition, LINK places the total number of variables in byte \>8212
and indicates the type of variable (number, string constant, numeric
variable, etc) into bytes \>200A-2019, one byte per parameter. Note that
this is different from CALL LINK in Extended Basic, that places the
parameter types at \>8300.

The parameter type values can be:
0: number
1: quoted string
2: numeric variable
3: string variable
4: numeric array element
5: string array element

All this makes parameter handling much easier for programs called from
Basic. It's not really a piece of cake though...

Once all parameters have been processed, LINK calls the linker via XML
\>21, with the program name in \>834A-834F and the name length in word
\>8350-8351. If the name was en empty string, the linker will attempt to
call again the previously executed program, if any.

Upon return, ar error is issued if the Cnd bit is set in the GPL status
byte. The error number is taken from byte \>8322. If no error occured
the subprogram removes all parameters from the value stack, if that
wasn't done by the called program, and checks that the Basic statement
terminates correctly (i.e \>8342 must contain \>00). Then it returns to
Basic.

###  CHARPAT

This subprogram is used to retrieve the pattern of a character, in the
range 32-159.

The first parameter must be a number or a numeric variable: this is the
number of the character of interest. The second parameter must be a
string variable. Upon return, it will contain an hexadecimal description
of the pattern, just like the ones used by the Basic CALL CHAR.

#### Example:

CALL CHARPAT(97,A\$)

A\$ now contains the pattern for character 97, the lower case A:
"00000038447C4444"

##  Programming tricks

And now, a few receipes on how to push that module a little further.
Mostly, they deal with the loader that is part of the Editor/Assembler
cartridge. There is no guaranty that these trick will work with a
different loader...

### Forcing loading at a given address

Normally, any code that was assembled as relocatable (RORG sections) can
be loaded anywhere in memory. That's the whole purpose of the
tagged-object file format. The cartridge's loader decides where to load
a given file according to the following algorithm:

- It first fills up the high memory expansion, from \>A000 to \>FFD7.
- If there is not enough room in there, it tries in the low memory
  expansion from \>2676 to \>3F38. Note that the symbol table grows
  backwards from \>3F38 and thus reduces the amount of space available
  in the low memory expansion by 8 bytes per DEFined label. REFerenced
  labels also occupy 8 bytes until the reference is resolved by an
  incoming DEF.

To achieve this task, the loader maintains four pointers in the low
memory expansion, at a location pointed at by label UTLTAB (normally
\>2022).

    Address Initially Usage
    >2022   >0000     Previous program address
    >2024   >A000     First free address in high memory: FSTHI
    >2026   >FFD7     Last free address in high memory: LSTHI
    >2028   >2676     First free address in low memory: FSTLOW
    >202A   >3F38     Last free address in low memory (bottom of symbol table): LSTLOW
    >20DA-20F9 Linking-loader worskpace.

Each time it loads a file in the high memory expansion, the loader
updates the value of FSTHI. Similarly, it updates FSTLOW when loading a
file in the low memory expansion, and LSTLOW when adding labels to the
symbol table (or removing labels, when a REF is solved by an incoming
DEF).

Now there may be times when we want to force loading of a relocatable
file at a given address in memory. We could reassembled it, using an
AORG statement that will force loading at a given address. But this is
not very convenient, especially if we don't have the source for that
file. A better alternative is to write a tiny file that will fool the
loader by altering the loading pointers with AORG statements. For
instance, the following file forces loading of the next relocatable file
at \>C000 in high memory.

* Load it before just before the reloc file
       AORG &gt;2024
       DATA &gt;C000      You can put any value here
       END

To force loading in the low memory expansion, we must first forbid
loading in the high memory. We could bump up FSTHI, but that would loose
the current value. It's therefore more clever to lower LSTHI:

       AORG &gt;2026
       DATA &gt;A000       No more loading in high memory
       DATA &gt;3000       Where to load in low memory (optional)
       END`
* This program enables loading in high memory again.
       AORG &gt;2026
       DATA &gt;FFD7
       END

###   Preventing "undefined symbol" errors

Before linking to a program, either from the RUN menu or from an
auto-start file, the loader scans the symbol table for undefined labels,
i.e. REFs that didn't get a matching DEF. In some cases, it could be an
advantage to bypass this function.

We could do this by just emptying the symbol table: it's a simple matter
of reseting LSTLOW to its initial value. However, this will only work
for auto-start files, as now any program name will cause a "program not
found" error. But we could adjust LSTLOW so that it leaves the first
label we loaded in the symbol table (or more than one if we wanted to,
the important point is to cut off any REF).

* This prevents the loader from complaining about unresolved REFs
       AORG &gt;202A     This is LSTLOW (bottom of symbol table)
       DATA &gt;3F30     Top is &gt;3F38. Substract 8 bytes per label
       END

###   Enabling illegal tags

The linking loader found in the cartridge only recognises a small subset
of the possible tags generated by the TI assembler. All others result in
an "illegal tag" error. We could write a set of loader improvements to
deal with the missing tags. Illegal tags D, E, G, and H all branch at
\>25D8. So do tags P and above. Tags K to O have no fixed branch point
since their vectors are overwritten by the first program loaded in the
low memory expansion. We could cause them to branch anywhere in the
loader between addressed \>23F8 and \>25F6, by just entering one byte
per tag containing the offset in words from \>23F8. To cause them all to
branch at \>25D8, we should use value \>F0. Then of course, we should
patch address \>25D8 to process the tags we are interested in.

####  Data segment

For instance, the assembler allows for a data segment (a PC concept, it
looks like this assembler was lifted from a PC assembler...). This
segment does not have to contain only data, all it means is that it will
be loaded at a different place than the rest of the program. It is often
convenient to load data at a location and program at another, since data
will generally be modified, whereas the program will only be read. For
instance, you may want to "burn" a program into an EPROM at address
\>4000, but you must obviously maintain your data in RAM. To do this,
enclose all your data between a DSEG and a DEND statement.

We must now deal with the following new tags at loading time:
**M**xxxx\$DATA 0000 At top of file: size of the data segment = xxxx.
**S**xxxx New loading address for data segment (xxxx=offset in data
segment).
**T**xxxx Reference to an address in the data segment from anywhere in
the program (xxxx=offset in data segment).
**W**xxxxAAAAAA0000 DEF of a label located in the data segment
(name=AAAAAA, value=xxxx).
**X**xxxxAAAAAA0001 Label REFerenced from inside the data segment
(name=AAAAAA, ptr to first ref=xxxx).

For the sake of simplicity, we're going to patch the tag checking
routine so that any tag above F branches to our routine. We'll process
the above tags or return to the loader for any other tag.

* It processes the previously illegal tags M, S, T, W and X
*
       AORG &gt;25D2             Patch the tag checking routine
       DATA &gt;000F             All tags above |F| will be illegal
       AORG &gt;25D8
       B    @NEWTAG           Patch reaction to illegal tags
        AORG &gt;2028
       DATA ENDPG             Patch FSTLOW since our program will be here
        AORG &gt;2676             Load this program just after the loader
NEWTAG CI   R3,&gt;001C          Tag value is in R3
       JEQ  LDADR             Tag |S| Loading address
       CI   R3,&gt;001D
       JEQ  ADDR              Tag |T| Address reference from CSEG
       CI   R3,&gt;0020
       JEQ  DATDEF            Tag |W| DEF in data segment
       CI   R3,&gt;0021
       JEQ  DATREF            Tag |X| REF in data segment
       CI   R3,&gt;0016
       JEQ  DATSIZ            Tag |M| Size of data segment
       CI   R3,&gt;0019          That was the old limit for legal tags
       JH   ILLEG
       B    *R11              Let the loader deal with these
 ILLEG  LI   R0,&gt;0A00          Error code 10 (&quot;Illegal tag&quot;)
       B    @&gt;2432            That&#39;s what we overwrote with our patch
 DATSIZ MOV  R8,@&gt;202C     |M| Save checksum
       BL   @&gt;2594            Read an integer from file
       INC  R0                Size of data segment
       ANDI R0,&gt;FFFE          Make it even
       A    @&gt;2028,R0         Add current FSTLOW
       C    R0,@&gt;202A         Compare with LSTLOW
       JHE  TOOBIG            Not enough room
       MOV  R0,@TEMP          Save for later
       LI   R9,6              Name is always &quot;$DATA &quot;
LP1    BL   @&gt;262E            Skip these 6 chars
       DEC  R9
       JNE  LP1
       BL   @&gt;2594            Read an integer from file
       MOV  0,0               Should be 0 for data segment
       JNE  ILLEG             Could be non-0 if CSEG: illegal tag
       MOV  @&gt;2028,@DSEGPT    Set data segment pointer
       MOV  @TEMP,@&gt;2028      New FSTLOW value
       B    @&gt;2400            Continue loading with next tag
 TOOBIG B    @&gt;2494            &quot;Memory overflow&quot; error
 
LDADR  MOV  R8,@&gt;202C     |S| New loading address in DSEG
       BL   @&gt;2594            Read an integer from file
       A    @DSEGPT,R0        Address in data segment
       B    @&gt;24B0            To address setting routine

ADDR   MOV  R8,@&gt;202C     |T| Reference to a DSEG-relative address
       BL   @&gt;2594            Read an integer from file
       A    @DSEGPT,R0        Address in data segment
       B    @&gt;24B6            To data loading

DATDEF MOV  R8,@&gt;202C     |W| DEF of a label located in DSEG
       BL   @&gt;2594            Read an integer from file
       A    @DSEGPT,R0        Address of the label in data segment
       BL   @&gt;2566            Create new entry in symbol table
       MOV  R0,@TEMP          Save label value for later
       BL   @&gt;2594            Read integer
       MOV  R0,R0             Should be 0 for DSEG
       JNE  SK1
       MOV  @TEMP,R0          Restore label value
       B    @&gt;2504            To DEF processing

DATREF MOV  R8,@&gt;202C     |X| REF to an external label from inside a DSEG
       BL   @&gt;2594            Read an integer from file
       A    @DSEGPT,R0        Address of the first reference in data segment
       BL   @&gt;2566            Create new entry in symbol table
       MOV  R0,@TEMP          Save ptr for later
       BL   @&gt;2594            Read integer
       MOV  R0,R0             Should be 0 for DSEG
       JNE  SK1
       MOV  @TEMP,R0          Restore ptr value
       B    @&gt;24C4            To REF processing
 SK1    B    @&gt;2400            Ignore other REFs/DEFs from other segments
 DSEGPT DATA 0                 DSEG pointer
TEMP   DATA 0                 Temporary buffer
 ENDPG  END

Loading that file enables the use of data segments: any file loaded
after that one may contain DSEG sections that will be loaded in the low
memory expansion.

####  Loading multiple segments

If you have a memory expansion device that performs bank switching, you
may want to extend the above concept to common segments. All you have to
do at programming time, is to split your program into (relatively)
independent units that you declare as segments by enclosing them in CSEG
segname .... instructions.

A segment loader would check all the tag M at the top of the file and
use them to arrange segments in the available banks. It should probably
create a segment location table somewhere in memory, that will be used
by your program to branch to a given segment.

The loader will use tags N, P, W and X pretty much in the same way as
described above for data segments. Except of course that is must read
the segment number at the end of each tag to determine which segment
must be accessed.

The nice thing is, you don't need to re-write an assembler: just use the
TI one and provide a special loader. Either by writing one from scratch,
or by patching the existing loader as described above.

###  Loading bias

Sometimes you would like to load a program in the whole range of the low
memory expansion. This is not possible, since it would cause the loader
to overwrite itself. And if we load the program elsewhere, say at
\>A000, then save it into a memory -image file that loads at \>2000, it
won't work properly since all addresses will be wrong.

Is there a solution (apart from writing a version of the loader that
runs in the high memory)? Well, we could fool the loader into using a
wrong offset value to adjust data when encountering a tag C. The problem
is that this will also affect the setting of a new loading address by
tag A, and the assembler issues a tag A at the beginning of each new
record. We must therefore disable tag A first:

* REFs are not allowed yet
       AORG &gt;266C
       DATA &gt;045F             Change the jump vector for tag |A| into tag |8|
        AORG &gt;20E4
       DATA &gt;2000             Change the content of loader&#39;s R5 (reloc offset)
SLOAD  EQU  &gt;2000             Where the program should run
        AORG &gt;A000             Where we want to load it temporarily
       NOP                    AORG won&#39;t be issued unless followed by some code
       RORG                   Ignored upon loading, but needed to assemble
SFIRST ....                   Put our program here`
SLAST  DATA 0                 This will be overwritten by the next file loaded
       END

The AORG \>A000 sets the loading pointer, that will remain unchanged
throughout the RORG section since tag A is now disabled. Since the
assembler disregards any AORG that is not followed by data, we need to
load a dummy NOP at address \>A000. Our program will therefore begin at
\>A002. Hence the final DATA 0, that will be overwritten by the next
program being loaded (since the loader added the size of our program to
FSTHI, which normally contains \>A000, not \>A002).

A big problem remains: we cannot use REFs in our program as the
reference solving routine uses R5 to find the next location to patch,
based on the current one (each occurence of a REFerenced label points to
the previous one). We could of course restore R5 at the end of our
program, but then the DEFined labels would be all wrong. And we cannot
place an AORG whithin the list of DEFs and REFs since the assembler
groups them all at the end of the file.

We must therefore use the REFerence solving routine to patch the loader.
To do this, we'll use one of the pre-defined labels, say SPCHRD which is
rarely used anyhow. We'll change its entry in the symbol table by
negating the first word, so that it appears to be an unresolved REF.
Then we'll change its value so that it points to R5 in the loader's
workspace. Then all we have to do is to DEFine SPCHRD in our program:
this value will be place in R5 by the REFerence solving routine. The
solving routine will then proceed with the address found in R5, then
with the address found at that address, until is finds a \>0000 value.
It is thus imperative that we stop the process by loading a \>0000 value
in the address pointed at by R5 (which is \>2000 in our example).

* The SAVE utility will handle it accordingly
        AORG &gt;266C
       DATA &gt;045F             Change the jump vector for tag &quot;A&quot; into tag &quot;8&quot;
        AORG &gt;20E4
       DATA &gt;2000             Change the content of loader&#39;s R5 (rel offset)
        AORG &gt;A000             Where we want to load it temporarily
       NOP                    AORG won&#39;t be issued unless followed by code
       RORG
       DEF  SLOAD             This DEF will be biased (i.e. &gt;2000)
       DEF  SPCHRD            Cancel bias now
       DEF  SFIRST,SLAST      These will have the real address (&gt;A002, etc)
       REF  SPCHWT            For test purposes: this REF will work normally
SLOAD
SFIRST
       ABS  @SLOAD            Silly test program: ABS @&gt;2000
       NEG  @SFIRST                               NEG @&gt;2000
       CLR  @SPCHWT                               CLR @&gt;9400
       B    *R11
SLAST
       DATA 0                 This will be overwritten by the next program loaded `
* This causes the DEF SPCHRD to cancel the load bias
       AORG &gt;3F78             Point at entry for label SPCHRD
       DATA &gt;ACB0             That&#39;s &quot;SP&quot; negated: this label is now an unresolved REF
       AORG &gt;3F7E             Point at value of SPCHRD
       DATA &gt;20E4             Link to the next occurence: this will be the loader&#39;s R5
        AORG &gt;2000
       DATA &gt;0000             Stop the reference solving (since R5 now contains &gt;2000)
SPCHRD EQU  &gt;A002             Value that will be placed in R5 (&gt;A000 +2 for the NOP)
        END`
* The liker will complain since we changed the value in &gt;2000 (&quot;program not found&quot;)
* Therefore load this program just after the above one to restore the flag value in &gt;2000
       AORG &gt;2000
       DATA &gt;A55A
       END

Load these two programs, then load the SAVE utility. It will save the
first program from the high memory expansion, into an EA5 file that will
load and run in the low memory expansion.

###  Extra assembler opcodes

By editing the ASSM1 file, I found out that the assembler can recognize
(and issue data for) a number of opcodes corresponding to machine
language instructions that are supposedly illegal with the TMS9900. This
is probably due to the fact that this assembler is able to produce
programs meant to run on other systems, such as the 990 minicomputer,
the TMS9940-based systems, TMS99000 or 9989 systems.

The opcodes in the first group are in format VII, which means they do
not take any operand.
laeco represent the status bits affected: **l**ogically higher,
**a**rithmetically greater, **e**qual, **c**arry and **o**verflow

    Value Opcode Status Meaning
    >0C00 CRI    laeco  Convert Real to Integer
    >0C01 CDI    laeco  Convert Decimal to Integer
    >0C02 NEGR   lae    Negate Real
    >0C03  -      -     (exists, but not encoded by the assembler)
    >0C04 CRE    laeco  Convert Real to Extended integer
    >0C05 CDE    laeco  Convert Decimal to Extended integer
    >0C06 CER    laeco  Convert Extended integer to Real
    >0C07 CED    laeco  Convert Extended integer to Decimal
    >0C08-0C0D    -     (exist, but not encoded by the assembler)
    >0C0E XIT           eXIT the floating point interpreter
    The opcodes in the second group are in format VI, i.e they take one operand in any of the general addressing modes.

    Value Opcode Status Meaning
    >0780 LDS           Long Distance Source
    >07C0 LDD           Long Distance Destination

    >0C40 AR     laeco  Add Real
    >0C80 CIR    laeco  Convert Integer to Real
    >0CC0 SR     laeco  Substract Real
    >0D00 MR     laeco  Multiply Real
    >0D40 DR     laeco  Divide Real
    >0D80 LR     lae    Load Real
    >0DC0 STR    lae    Store Real
    >0E00 -            (not encoded by the assembler assembler, but some exist)
    >0E40 AD     laeco  Add Decimal
    >0E80 CID    laeco  Convert Integer to Decimal
    >0EC0 SD     laeco  Substract Decimal
    >0F00 MD     laeco  Multiply Decimal
    >0F40 DD     laeco  Divide Decimal
    >0F80 LD     lae    Load Decimal
    >0FC0 STD    lae    Store Decimal

All opcodes dealing with Reals are meant for TMS99000-based systems that
use a 99110A CPU and for the 9989. They are designed to provide a
floating point package.

The Decimal and Extended integer opcodes are found on the 9989. They
provide an easy-to-use BCD interface.

LDS and LDD are meant for 990/10 mainframe systems, for 99000-based
systems and for the 9989. They are designed to support the TMS99610
memory mapper, to allow easy access from one page to another without the
need for the user to switch pages.
Finally, there are a few more:

    Value Opcode Format Meaning
    >2C00 DCA    IX     Decimal Correct for Addition (= XOP 0)
    >2C40 DCS    IX     Decimal Correct for Substraction (=XOP 1)
    >2C80 LIIM   IX     Load Immediate Interrupt Mask (= XOP 2)
    >0320 LMF    X      Load Memory-map File

The first three are aliases for XOP instructions. They are meant for use
with a 9940 CPU. This CPU only allows 4 levels of interrupts and
therefore cannot use LIMI (which allows 16 levels). Hence the special
LIIM instructions that takes a number from 0 to 3 as an argument. DCA
and DCS take one argument, in any of the general addressing mode. You
can use them as shortcuts for XOP 0 and XOP 1 on the TI-99/4A, if you
feel like it.

LMF takes two operands: a register and a number that can be 0 or 1. It
is used by the TMS99000, but I don't know what for.

####  Not assembled opcodes

The assembler will not produce the following opcodes, although they can
be used on the 9989 and often also on the 99000. Formats I-IX are the
[standard formats](assembly.htm#Encoding%20format) we know, formats
X-XXI are unique to these special opcodes and are only used by the
TI990/40 minicomputer.
laeco represent the status bits affected: **l**ogically higher,
**a**rithmetically greater, **e**qual, **c**arry and **o**verflow

    Value Opcode Format Status Meaning
    >0000-001B   -            (no opcodes)
    >001C SRAM   XIII  laec   Shift Right Arithmetic Multiple
    >001D SLAM   XIII  laeco  Shift Left Arithmetic Multiple
    >001E RTO    XI      e    Right Test for One
    >001F LTO    XI      e    Left Test for One
    >0020 CNTO   XI      e    Count Ones
    >0021 SLSL   XX      e    Search LiSt Logical address
    >0022 SLSP   XX      e    Search LiSt Physical address
    >0023 BDC    XI    lae o  Binary to Decimal ascii Conversion
    >0024 DBC    XI    lae o  Decimal ascii to Binary Conversion
    >0025 SWPM   XI    lae    SWaP Multiple precision
    >0026 XORM   XI    lae    eXclusive OR Multiple precision
    >0027 ORM    XI    lae    OR Multiple precision
    >0028 ANDM   XI    lae    AND Multiple precision
    >0029 AM     XI    lae    Add Multiple
    >002A SM     XI    lae    Substract Multiple
    >002B MOVA   XIX   lae    MOVe Address
    >002C -                   no opcode
    >002D EMD    VII   (all)  Execute Micro Diagnostic
    >002E EINT   VII          Enable INTerrupts
    >002F DINT   VII          Disable INTerrupts
    >0030 STPC   VIII         Store Program Counter
    >0040 CS     XII   lae    Compare String
    >0050 SEQB   XII   lae    Search string for EQual Byte
    >0060 MOVS   XII   lae    MOVe String
    >0070 LIM    VIII  (mask) Load Interrupt Mask from register
    >0080 LST    VIII  (all)  Load Status from register
    >0090 LWP    VIII         Load Workspace Pointer from register
    >00A0 LCS    VIII         Load writable Control Store
    >00B0 BLSK   VIII         Branch immediate and push Link to StacK
    >00C0 MVSR   XII   lae    MOVe String Reverse
    >00D0 MVSK   XII   lae    MOVe String from StacK
    >00E0 POPS   XII   lae    POP String from stack
    >00F0 PSHS   XII   lae    PuSH String to stack
    >0100 EVAD   VI           EValuate ADdress instruction
    >0140 BIND   VI           Branch INDirect
    >0180 DIVS   VI    lea o  Divide-Signed
    >01C0 MPYS   VI    lae    Multiply-Signed
    >0301 CR     ??    ?????  Compare Reals
    >0302 MM     ??    ?????  Multiply Multiple
    >0303-031F   -            (no opcodes)

    >03F0 EP     XXI          Extended Precision

    >0C03 NEGD   VII   lae    Negate Decimal
    >0C08 NRM    XI    lae    Normalize
    >0C09 TMB    XIV     e    Test Memory Bit
    >0C0A TCMB   XIV     e    Test and Clear Memory Bit
    >0C0B TSMB   XIV     e    Test and Set Memory Bit
    >0C0C SRJ    XVII         Substract from Register and Jump
    >0C0D ARJ    XVII         Add to register and Jump
    >0C0F -                   no opcode
    >0C10 INSF   XVI   lae    Insert Field
    >0C20 XV     XVI   lae    eXtract Value
    >0C30 XF     XVI   lae    eXtract Field

    >0E00 IOF    XV           Invert Order of Field
    >0E01 SNEB   XII   lae    Search string for Not Equal Byte
    >0E02 CRC    XII     e    Cyclic Redundancy Code calculation
    >0E03 TS     XII   lae    Translate String
    >0E04-0E3F   -            (no opcodes)
If you have any insight on what these opcodes are good at, please [let
me know](feedback.htm#questions).

##  File formats

### Memory-image EA5 files

These "program" files contain a mere dump of the memory content. There
is only a 3-word header that contains instructions for the loader.
Normally, this will be option 5 of the Editor/Assembler cartridge, hence
the name EA5. But there are other loaders around: you can even
[download](download.htm#MILD%20and%20MISS) one from this website (MILD).

The header structure is the following:

|         |                                                       |
|---------|-------------------------------------------------------|
| Address | Content                                               |
| \>0000  | Flag. \>0000 = last file. \>FFFF = load one more file |
| \>0002  | Address where to load the program.                    |
| \>0004  | Number of bytes to load (not counting these 6).       |

The loader loads the specified number of bytes, taken directly after to
the header, to the specified address in memory. If the flag word in
\>FFFF, the loader then loads another file whose name is coined by
incrementing by 1 the last character of the current file name. This
process goes on until a file with a \>0000 flag is loaded. At this
point, the loader executes the program by performing a `BL` to the
address where the first program was loaded.

The workspace upon entry will be \>20BA with the Editor/Assembler
cartridge, but the program should not take this for granted since other
loaders may use a different workspace. Similarly, it may be wise for a
program to properly setup the VDP memory, rather than to assume that the
Editor/Assembler cartridge has already done it.

A program can return to the caller with` B *R11`, or reset the TI-99/4A
with `BLWP @>0000`.

###  Tagged-object code files

These Div/Fix 80 files are produced by the assembler and used by the
linking loader to place a program in memory. They are more bulky and
slow to load than EA5 files, but have two main advantages:

- They can be loaded anywhere into memory, the loader just has to find a
  place where there is enough room for the whole file. By contrast EA5
  files must be loaded at a fixed address. Note that it is also possible
  to force loading of an object file, or part of it, at a fixed address:
  the AORG assembly instruction provides for it.
- It is possible to load several files and to link them together by the
  way of DEF and REF statements in assembly. The loader uses the values
  DEFined in a file to resolved the REFerences in the others.

All this is achieved by interlacing the data words with special
characters called "tags". A tag is nothing else than an instruction for
the loader. Almost all tags are followed with a word of data, sometimes
by a string or an additional word. There are two file formats,
compressed and uncompressed, determined by the C option of the
assembler. In compressed files, data words are passed as integers, and
no checksum nor record number is issued. In uncompressed mode, the
numbered are "spelled out" as 4-character hexadecimal strings. This has
the advantage that the file can be edited with the Editor without having
to re-assemble the source file.

Below are all the tags I'm aware of. Not all of them can be issued by
the assembler provided with the Editor/Assembler cartridge.


$$$$$$$$: its name (empty unless renamed with the IDT instruction).
name
#### offset of the last address referencing this label. At end of
file.
name
#### last address referencing this label. At end of file.
name
#### offset of the label in the segment. At end of file.
label.
$$$$$$: label name, #### label value (e.g. absolute address). At end of
file.
7.
pointer)
REF.
number.
`00tt` or `000t`
segment.
But the TI loader uses `tt `as the next tag (`t`
in uncompressed mode)
`####$BLANK0001`
`####$$$$$$0xxx`
CSEG
of the blank common segment (CSEG with no name),
or of the common segment named $$$$$$, number xxx assigned by
assembler.
memory.
name
`####$$$$$$0xxx`
CSEG
label name
#### offset of the label in segment xxx. At end of file.
`####$$$$$$0xxx`
CSEG
name
#### offset of the last address referencing this label. At end of
file.
name
`####0xxx`
CSEG
name
xxx: segment number.
any.

\* These tags are not accepted by the standard loader provided in the
Editor/Assembler cartridge.
! These tags are not issued by the standard assembler, as far as I
know.
\#### In compressed mode: 2-byte integer. In uncompressed mode: string
of 4 hex digits 0-9, A-F. Ditto for nnnn.
0xxx Segment number, from 0 (DSEG) to 127. Compressed: 2 bytes,
uncompressed: 4 hex digits.
\$ Any ascii character. Not affected by compression mode.

####  Segments

Tags **0** and **M** are used by the loader to find a free slot in
memory where to load a given segment. The total size of the segments is
passed as an argument together with the segment name. Note that the
names of the data and the "blank-common" segments are predefined (\$DATA
and \$BLANK), whereas the name of the program segment can be defined
with the IDT assembly instruction (by default, it's a string of 8
spaces). Other common segments can be specified by placing their name
after the CSEG instruction in the assembly program. Tag M is followed by
the number of the segment: \>0000 for the data segment, \>0001 for the
blank common segment (i.e. when no name is specified after CSEG), and
higher numbers for all other common segments.

There can thus be at least three type of relocatable segments: program,
data and common segments that can be loaded at different memory
locations. All the PSEG sections in a file will be grouped together
inside the program segment, all DSEG inside the data segment and all
CSEG inside the corresponding common segments. AORG sections are always
loaded at the specified address and the loader therefore has no need to
know their size.

Note that the loader in the Editor/Assembler cartridge cannot handle the
data nor the common segments.

####  Loading data

Tags **9**, **A**, **S** and **P** are used to specify a loading
address. Tag 9 passes the address as such. The others pass an offset
within the program, data or common segment respectively. The loader must
add the address of the beginning of that segment to the offset to obtain
the actual address. Note that tag P comprises an additional number that
specifies the number of the common segment to be used.

The loader in the cartridge only recognises tag 9 and A. It loads
relocatable segments in the high memory expansion first, then in the low
memory expansion and maintains a pointer to the next available space,
that will be used as a load offset for the next file. Tag **D** can be
used to override this offset by a user-specified value. This tag isn't
produced by the assembler, but is meant for use when editing the code
file.

Tag **B** copies data as such into memory. Most of a program will
consists of data that can be passed by tag B. The only exceptions are
instruction whose operand is a label located in a relocatable segment,
whether program, data or common. E.g. `CLR @COUNT` or `BL @SUB1`,
assuming COUNT is in the data segment and SUB1 in the program segment.
The assembler does not know where these segments will be located, but it
knows the offset of a label with respect to the beginning of the
segment. Therefore, it issues a tag **C**, **T** or **N** which tells
the loader to add the address of the program, data or common segment
respectively to the data word before to load it into memory. Here also,
tag N specifies the number of the common segment to use.

####  Labels

Tag **6** is used to define a label. The value of the label and its name
are passed together with the tag. Such labels could be addresses in an
absolute section, or values defined with an EQU statement. By contrast,
tag **5** and **W** are used to specify labels corresponding to an
offset in the program, data or common segment (W is shared by data and
common segment: an extra word is passed to determine which segment must
be used). Here again, the loader adds the base address of the adequate
segment to specified offset and places the label in the symbol table,
thereby possibly solving any pre-existing REFs.

Tag **4**, **3** and **X** are used to REFerence external labels (i.e.
labels DEFined in another file) from inside an absolute section, a
program, a data or a common segment (X is shared by data and common: an
extra word containing the segment number determines which is which). The
accompanying data corresponds to the offset (or the absolute address
with tag 4) of the last place in the segment where this label is used.
If the loader knows the value of this label, it places it at the pointed
location. Otherwise it places the label in the symbol table (and negates
the first 2 chars) and waits for a DEF to the same label to arrive, by
which time it will solve the REF.

Since it would use up a lot of memory to have a tag for each memory word
that uses an external label, the assembler uses a special trick. There
is only one REF per label (per file), and it points to the last address
where the label is used. The content of this address, that will be
replaced with the label value, contains a pointer to the previous
address where the label was used. This pointer is an absolute address
with tag 4, an offset with tag 3 and X. That address in turn contains a
pointer to the previous one, etc. The last address contains a \>0000
word with a tag B to mark the end of the chain. All the loader has to do
is to walk the chain, replacing each pointer with the value of the
label.

            REF  TEST
            IDT  &#39;PLAYBOYS&#39;  0000CPLAYBOYS   (size and name of PSEG)
&gt;0000       CLR  @TEST       B04E0B0000&lt;---+ (end of REF chain)
&gt;0004       SETO @TEST       B0720C0002----+
&gt;0008       DATA TEST        C0006--^   &lt;---+
&gt;000A       ABS  @TEST       B0760C0008-----+  &lt;--+
            END              3000CTEST  F---------+ points to last use of TEST
Tag **V** is issued by the SREF instruction and
is equivalent to tag 3. The only difference is that it will not generate
an "undefined symbol" error number \>0D if no matching DEF was provided
by the time the program is launched. This can be useful if your program
comprises optional modules. All you must do is to check the first
occurence of this label in your program to see if a value was provided
by the loader: if it contains \>0000 the optional module was not loaded.
Note that this tag is not supported by the standard loader.

Tag **Y** does the same for SREFs included in an absolute section, tag
**Z** for SREF in a DSEG or CSEG segment.

* PRINT is a procedure in a optional extra file
       SREF PRINT      Reference it, but don&#39;t check for a DEF `
       MOV  @HERE+2,R0
       JEQ  NOPE       The extra file was not loaded
HERE   BL   @PRINT     The second word is &gt;0000 before linking
NOPE   ...
              MOV  @HERE+2,R0
       JEQ  $+4         Skip if not loaded
       BL   @PRINT      The 2nd word is not &gt;0000: it points at HERE+2
       ...
Tag **U** is an oddball. It is issued by the
assembly instruction LOAD, whose operand is a label. Contrarily to the
other tags dealing with labels, that are grouped at the end of the file,
tag U appears in the program at the position it was inserted in the
source file. The value of the LOADed label is always \>0000. Appearantly
this is meant as a dummy DEF, used to force loading of a label in the
symbol table (possibly to implement \#ifdef and \#ifnotdef statements?).
It is not supported by the standard loader anyway (but it could be
useful to implement a fancy loader and pass loading commands without
having to re-write the assembler...).

Tag **E** is another weird one. As far as I
know, it is not issued by the assembler and is not recognized by the
loader. It serves to add an absolute value to a REF symbol. It is
followed by two words of data: the second is to value to be added, the
first one is the number of the REF symbol to be modified. REF symbols
are counted from 0, in the order they appear in the code file, whether
the are preceded by a tag 3, 4, V, X, or Y. When resolving the REF
chain, the specifed value should be added to the DEFined one.

Tag **K** is an external macro REF (whatever
this means). It has the same syntax as tag 3 but is not issued by the
assembler, not recognized by the standard loader.

####  Autostart

Tag **1** and **2** are used by auto-start files. The loader saves the
address they provide in word \>2022 (with tag 2, it adds the address of
the program segment to the specified offset). Once loading is completed,
i.e. a tag : is encountered, the loader begins execution at this
address. That is, provided there are no REF labels that were not solved
by a matching DEF.

To generate a tag 1 or 2, you place a label in the operand field after
the END instruction: `END START`

####  Record info

Tag **F** marks the end of each record, any following byte in this
record is ignored. With uncompressed files the last 4 bytes in the
record contain a decimal record number.

Tag **:** signals the end of the file. Any extra byte on this record are
ignored, so is any additional record.

Tag **7** is used to verify the integrity of a record. Both the
assembler and the loader perform the same calculation on each record:
they add up all bytes in the record, upto and including the tag 7
itself. Then the assembler negates the resulting word and passes it with
a tag 7 in uncompressed files. The loader just adds this value to the
checksum it has calculated: if the result is not zero the record has
been tempered with and a loading error occurs.

Tag **8** is not issued by the assembler. It is useful for us as it
allows to comment out any tagged word. Most often this will be to
replace tag 7 and prevent verification of the checksum.

Tag **I** is issued by the SYMT directive, which does not work with the
TI assembler: it includes the program ID in the output file. The loader
recognises it, and just skips the next 8 characters.

####  Oddballs

The following tags are not issued by the assembler, as far as I know.
They were meant for other assembler, such as the model 990 computer
assembler.

Tag **G**, **H** and **J** are used to include a copy of the whole
symbol table within the output file. This is triggered by the SYMT
directive, which is not supported by the assembler included in the
Editor/Assembler module. Tag G would be used to list relocatable
symbols, tag H for absolute symbols and tag J for data or common segment
symbols.

The loader does recognise tag J, but in a very stange way: it uses the
second byte of data as a tag (or the last hex digit with uncompressed
files). I have no idea whether this is of any use or if it's just a bug,
since it does not match the expected syntax for tag J.

Tag **Q** references a Cobol segment. It is followed by the record
offset and the Cobol segment number.

Tag **R** specifies a repeat count. It is followed by a value and the
number of repeats.

I do not know the meanings of the remaining tags: **L**, **O**, nor of
any tag beyond Z, if any.

If you can provide any information on these, by all means [let me
know](feedback.htm#questions).
Revision 1. 6/22/99 Ok to release
Revision II. 9/5/99 Added non-TI99 opcodes. Added DF80 tag D + K. CSEG
discussed.

Revision III. 1/7/01 Added tags E,G,H,K,Q,R,Y,Z.

[Back to the TI-99/4A Tech Pages](titechpages.md)
