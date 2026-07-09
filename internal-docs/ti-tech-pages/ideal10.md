# IDEAL 1.0

Click [here](ideal.md) for a more recent version.
### Introduction

The [IDE card](ide.md) operating system loaded inside the SRAM is
called IDEAL for "IDE Access Layer". It contains all the routines that
let you access the IDE drives, the clock, the card SRAM and even
configure IDEAL itself.

Version 1.0 turns your hard disk into a huge collection of "virtual
floppies" that can either be accessed by name, or by "inserting" them
into 8 virtual drives. It is fully compatible with the TI floppy disk
format, and therefore has the same limitations: only 1440 sectors and
127 files per disk, at most 76 clusters per file, 256 byte per sector,
etc. I'm planning a new version that will not suffer from these
limitations and will co-exist with the TI-compatible one, but this will
take time...

You'll find some improvements on the TI format though. First of all, I
integrated most of the new features introduced in the Horizon Ramdisk
operating system (a good part of my code is shamelessly lifted from ROS
8.14 and from the TI-controller card ROMs, but why re-invent the
wheel?). Thus, you'll find extra opcodes to delete a record, to load an
assembly, Basic or cartridge file, an autostart feature, and the
possibility to place a file data buffer in CPU memory instead of VDP
memory.

IDEAL lets you create directories and subdirectories. In fact, a
directory is nothing more than an empty file that points to another
virtual floppy. This lets you organise your virtual floppies in a
hierarchical manner, while remaining compatible with a floppy disk
controller (although of course, directories won't be more than empty
files if you copy them to a real floppy).

IDEAL provides you with the possibility to encrypt a file, or a whole
virtual floppy, using the Blowfish algorithm. You also have a direct
access to the encryption engine, so you can encrypt your own data.

Optionally, IDEAL will time-stamp each file and/or disk with a creation
date and a date of last modification. You can also access the clock
directly to set the time or an alarm, and to retrieve the current date
and time, either as numbers or as pre-formatted text strings.

The standard catalog function works just as with the TI floppy
controller, but an extended catalog is also provided that returns extra
information on the files (such as the time stamps) and lets you perform
some file management functions. It also lets you edit a floppy
sector-wise from Basic.

Finally, IDEAL has the capability of configuring itself, so you can
tailor it to your needs and preferences.

[Getting started
](#Loading%20IDEAL)[Overview
](#Overview)[Subdirectories
](#Subdirectories)[Encryption
](#Encryption)[Extended catalog
](#Catalog)[Sector-wise access
](#Sector%20access)[Editing the floppy collection
](#Collection)[Accessing the clock
](#clock)[System configuration
](#Configuration)[Drive configuration
](#Drive%20ID)[Direct hard-drive access](#hard-drive%20access)

[DSRs
Subprograms
](#DSRs)[Modifications to the TI format](#New%20format)

## Getting started

This section discusses how to instal IDEAL in your IDE card and how to
configure it for the first time. When reading this document for the
first time you can just jump ahead to the [Overview](#Overview) chapter.

Installing IDEAL involves the following steps

1.  Download the program: [IDEAL.ZIP](ideal.zip) (~45 kB) and expand it
    on your PC.
2.  Follow the instructcions in the README.TXT file to transfer the disk
    file to your TI-99/4A.
3.  Load IDEAL into the SRAM. Optionally load a bootstrap into the clock
    RAM.
4.  Set the proper time and date (assuming there is a clock on the
    card).
5.  Copy the IDEAL files to your hard-drive, for the bootstrap to load
    them at power-up time.
6.  Enter TI-Basic and run `NEWCFG `(after `CALL FILES(1)` ) to
    configure the system as you wish.

#### Files to be found in the ".dsk" file

    File                Type       Usage
    IDEAL/A ... IDEAL/M Program    IDEAL operating system to be loaded into the SRAM
    IDEAL/Z             Program    Bootstrap (resides in clock RAM)
    IDELOAD             Program    System loader (loads IDEAL + boostrap + config)
    IDE-LD2/O           DF80       Patch for the above, for non-standard cards
    IDE-LD2/S           DV80       Source for the patch
    IDEAL/GK            Program    German 128K Gram-Karte access routine
    IDEAL/PG            Program    P-Gram+ card access routine
    IDE_GC/S            DV80       Demo: sources for IDEAL/GK and IDEAL/PG
    NEWCFG              TI-Basic   Configures IDEAL (Do CALL FILES(1) before OLD)
    SETIME              TI-Basic   Sets the time and date
    NOLOAD              Program    Placeholder file for Mini-IDEAL

    CONFIG              TI-Basic   Demo: Reads the system configuration
    FLOP                TI-Basic   Demo: Embryonic virtual-floppy manager
    TRASH               TI-Basic   Demo: Recovers deleted floppies
    CAT                 TI-Basic   Demo: Extended disk catalog
    BLOW                TI-Basic   Demo: Direct access to the encryption engine
    CLOCK               TI-Basic   Demo: Access the real-time-clock DSR
    SECTORS             TI-Basic   Demo: Direct access to hard-drive sectors
    VAR2FIX             TI-Basic   Copy a DV80 file into a DF80 (for IDE-LD2/O)

NB Don't try to run the `IDEAL/x` files with an EA5 loader: they must be
loaded into the card SRAM and only `IDELOAD` knows how to do this
properly.

**Installing the IDE card**

First pick up a CRU address for the card. There should not be any other
card with this CRU in your system. A convenient CRU to use is \>1000, so
that the IDE card will be scanned before the floppy disk controller
(whose CRU is \>1100). If you don't know what CRU addresses are
available in your system, you can load my [Module
Explorer](download.htm#modexp) program, press Ctrl-7, then Fctn-3 to
enter the Header analyser. The arrow keys let you travel among the
installed cards and their subprograms, DSRs, etc.

Once you decided on the CRU address, compose it on the DIP-switch that's
on the card: all switches closed for \>1000, all open for \>1F00, i.e.
you are writing the binary value for the second digit in the CRU
address.

The fifth DIP-switch lets you decide whether the clock RAM or the SRAM
should appear at \>4000 when the power is first turned on. If you have a
battery-backed SRAM open the switch, otherwise close it and load the
bootstrap into the clock.

Make sure the main switch is off, so any garbage in the SRAM (or clock
RAM) won't prevent the system from booting.

Connect your hard drive(s) to the card via a standard IDE cable. Then
carefully insert the card in the PE-box.

Power-up the system: first the PE-box (and hard-drive), then the
console. If your drive takes a few seconds to spin up, it's a good
practice to wait till it's ready before you turn the console on. Now
flip the main switch on the card, so that it becomes active.

#### Loading IDEAL into the SRAM

The first thing to do is to load the system into the SRAM so you can
access the hard disk. You can also load a bootstrap into the clock RAM.

N.B. If you have a Gram device, you may want to first replace the file
`IDEAL/G` with the one corresponding to your Gram card: just rename
`IDEAL/GK` for the german 128K Gram Karte, or `IDEAL/PG` for the PGram
card. The original `IDEAL/G` file contains both routines, but calls the
one for the Gram Karte. If you have a different Gram device, you will
need to write a custom routine if you want IDEAL to handle it (which is
optional anyhow). See [below](#Gcard%20routine) for a description of the
required routine.

Now load the program `IDELOAD` using Editor/Assembler option 5, or with
a similar loader (such as Funnelweb). The program first prompts you for
the CRU of your IDE card. Enter the value that you set with the
DIP-switch on the card. Provided the card is found and the hard-drives
answers properly, `IDELOAD` will ask whether you want to load IDEAL, the
bootstrap and/or the configuration. Finally, you must enter the number
of the drive where the files are to be found (normally, this will be
`DSK1`). At any time, you can press \ to go back to the previous
step. Loading begins after you entered the drive number.

If you elected to load the configuration, or if you loaded IDEAL,
`IDELOAD `will check the clock memory for valid configuration data, and
ask you whether you want to keep it. If you decline, or if none was
found, the program checks a reserved area on the hard drive, where a
backup of then configuration is kept. If a valid configuration is found
there, you again have the choice to use it or not. If you don't, or if
none was found, the programs asks you if you want to load the
configuration from the file `IDEAL/M`. The latter option is not offered
if you have just loaded IDEAL, since in this case the configuration has
already been loaded from file.

The bootstrap is the file `IDEAL/Z`. It's a small program that resides
into the clock memory and loads IDEAL from your hard drive when you
power-up your system. Of course, this implies that you have copied (or
intend to copy) IDEAL on your hard-drive.

Once loading is completed you have the option to leave the bootstrap in
control. This means that the next time you hit \, the bootstrap
will attempt to load IDEAL from the hard drive. Obviously, you should
only select this option if you have already copied IDEAL on the hard
drive (i.e. if you are reinstalling the system). Answer by Y or N, or
press \ to return to the drive number prompt.

Technical note

If you made changes to my schematics and are using different CRU bits,
you will need to patch `IDELOAD `so that it modifies the system
accordingly. See [below](#non-standard) for details.

#### Setting the time and date

At this point, you may want to open the `CLOCK `DSR and set up the
correct time and date. Refer to the clock section [below](#clock).

Alternatively, you can run the TI-Basic program `SETIME `that will walk
you through the process.

#### Copying IDEAL on the hard disk

If you are installing IDEAL for the first time, it's a good idea to make
a copy of it on the hard drive. This will make possible for the
bootstrap to automatically load IDEAL when the system powers up. It is
not required if you have a battery-backed SRAM, but it won't hurt to do
it anyhow.

Create a new virtual floppy either with a disk manager "format" command
or by using the `IDE.FLOPPIES` pseudo-file:

    OPEN #1:"IDE.FLOPPIES", INTERNAL, VARIABLE 80, APPEND
    PRINT #1:"IDEAL",720
    CLOSE #1
    CALL CD.5..IDEAL

N.B. The `CALL `loads the virtual floppy into a virtual drive called
`DSK5` (by default IDEAL comes with drives DSK4, 5, 6, 7, 8, 9, A and B
installed).

Now use a disk manager to copy all the system files `IDEAL/A `through
`IDEAL/M` to `DSK5`.

IDEAL will be automatically reloaded in the following circumstances:

If DIP-switch 5 is in the "clock" position and a valid copy of IDEAL is
not present in the SRAM when you power-up the TI-99/4A, the bootstrap
will load IDEAL from the hard drive.

When you reset the TI-99/4A, IDEAL checks itself for integrity (so as
not to slow down the reset, only a small part of page 0 is tested). If
any corruption is detected, the boostrap is called and IDEAL is reloaded
from disk. If no boostrap is present in the clock memory, IDEAL makes
itself invisible to the system by clearing word \>4000 in the SRAM.

You can force a reload by holding down Ctrl-Enter while the TI-99/4A is
restarting.

#### Configuring your system

The TI-Basic program called `NEWCFG `can be used to set your system
configuration after you installed it. Note that you must perform
`CALL FILES(1)` before you load this program (it's big and slow...). The
program starts by asking for a password: for the moment being just hit
\. Later on you can use option 6 to set your own password. The
other options let you configure IDEAL.

Alternatively, you can update the configuration yourself via the file
`IDE.CONFIG`, which is what `NEWCFG `is doing anyway. Refer to the
corresponding section [below](#Configuration).

In any case, the parameters that you should set are:

Hardware description (if non-standard)

User options

Name of the virtual drives

Location of the system files

Optionally: autostart program, location of the Extended Basic cartridge.

Now you are ready to roll... A few more technical details follow, but on
first reading you can just skip ahead to the "[Overview](#Overview)"
section.

#### Backup issues

In addition to these options, IDEAL also retains some important internal
pointers, such as the root of your floppy disk collection (arranged as
an [AVL tree](avltrees.md)) or pointers to each trash file (arranged as
double-linked lists). Obviously, you can't afford loosing this
information, so several layers of backup have been implemented in IDEAL
1.0:

1\) If you have a battery-backed SRAM, the configuration info should be
safe at any time, even when the PE-box is turned off. Unless of course
the battery runs out...

2\) If your clock chip has on-board memory, the configuration will
automatically be saved into it after any change. When you reload IDEAL
(either with `IDELOAD`, or through the boostrap), the configuration is
copied from the clock memory into the SRAM.

3\) An additional backup is maintained on the hard-drive itself, in a
reserved area that is not part of your virtual floppy collection
(namely, hard-drive sector 0). This backup is updated each time you
`CLOSE `the file `IDE.CONFIG `after a modification. If you reset the
computer before closing the file, any change you made will be saved into
the clock RAM but not on disk!

Note: This backup is only used if a valid configuration cannot be found
in the clock memory when IDEAL is loaded. To invalidate it and use the
disk backup instead, use the `CLOCK `DSR to alter page 55 in the clock
memory:

    100 OPEN #1:"CLOCK", INTERNAL, FIXED 128, RELATIVE
    110 PRINT #1, REC(100+55): "0123456789abcdef0123456789abcdef"
    120 CLOSE #1

Then force a system reload by holding down Ctrl-Enter while you reset
the TI-99/4A.

4\) If you feel that you need an additional level of backup, you can
save the configuration present in the hard drive onto a floppy. This
implies that sector-wise access of the hard drive is enabled (you can
always turn it off later).

    100 OPEN #1: "IDE.SECTORS", INTERNAL, FIXED 129, RELATIVE
    110 PRINT #1, REC 0: 0,0
    120 INPUT #1: PART1$,PART2$,PART3$,PART4$
    130 CLOSE #1
    140 OPEN #2:"DSK1.BACKUP",INTERNAL, FIXED 129, OUTPUT
    150 PRINT #2: PART1$,PART2$,PART3$,PART4$
    160 CLOSE #2

To restore a lost configuration from such a backup just reverse the
process, then invalidate the clock memory and force a reload as
described above.

####  Mini-IDEAL

IDEAL version 1.0 is quite a big program. Currently, it uses 10 pages in
the SRAM, for a total of about 50 Kbytes. That's not a problem if you
have a 128K or a 512K SRAM on your card, but if you went the cheap way
and have only 32K you are in trouble. Not to worry though: there is a
way to skim down IDEAL, so it would fit in four pages. This may also be
usefull if you want to use a lot of RAMBO pages.

Here are the SRAM pages used by IDEAL:

               Page 0               Page 1               Page 2
    >4000+-----------------+  +-----------------+  +-----------------+
         | Registers       |  | xxxxxxxxxxxxxxx |  | xxxxxxxxxxxxxxx |
    >4000+-----------------+  +-----------------+  +-----------------+
         | Entry points    |  | CLOCK DSR       |  | Char patterns   |
         | Config data     |  | RAMBO manager   |  | for LD          |
         | Common          |  | ISR             |  |                 |
         | subroutines     |  |                 |  |                 |
    >5000+-----------------+  +-----------------+  +-----------------+
         | Sub >10 >14 >15 |  | Opcodes 0 to 4  |  | Autostart       |
         | Dir-files subs  |  | 8 and 9         |  | LD DSR          |
         |                 |  |                 |  | Opcodes >A to >C|
         |                 |  |                 |  | Sub >11 >12 >13 |
         | Buffers (>5B00) |  |                 |  | Custom subs     |
         +-----------------+  +-----------------+  +-----------------+

               Page 3               Page 4               Page 5               Page 6
    >4000+-----------------+  +-----------------+  +-----------------+  +-----------------+
         |xxxxxxxxxxxxxxx  |  | xxxxxxxxxxxxxxx |  | xxxxxxxxxxxxxxx |  | xxxxxxxxxxxxxxx |
    >4100+-----------------+  +-----------------+  +-----------------+  +-----------------+
         |    not used     |  | Number Pi       |  | Encryption      |  |    not used     |
         |                 |  | for Blowfish    |  | key             |  |                 |
    >5000+-----------------+  +-----------------+  +-----------------+  +-----------------+
         | Floppy collec   |  | Configuration   |  | Blowfish        |  | Trash management|
         | management      |  | management      |  | encryption      |  | IDE.TRASHx      |
         | IDE.FLOPPIES    |  | Catalog         |  | IDE.BLOWxxx     |  |                 |
         |                 |  | Extended cat    |  |                 |  |                 |
         | Buffers         |  | Buffers         |  |                 |  |                 |
         +-----------------+  +-----------------+  +-----------------+  +-----------------+

              Next page            Next page            Next page
    >4000+-----------------+  +-----------------+  +-----------------+
         | xxxxxxxxxxxxxxx |  | xxxxxxxxxxxxxxx |  | xxxxxxxxxxxxxxx |
    >4000+-----------------+  +-----------------+  +-----------------+
         |    not used     |  |    not used     |  |    not used     |
         |                 |  |                 |  |                 |
    >5000+-----------------+  +-----------------+  +-----------------+   (more pages)
         | Buffering subs  |  | Buffering subs  |  | Buffering subs  |   (if needed )
    >5400+ Opened files    |  | Opened files    |  | Opened files    |
         | buffers         |  | buffers         |  | buffers         |
         | (5 files)       |  | (5 files)       |  | (5 files)       |
         |                 |  |                 |  |                 |
         +-----------------+  +-----------------+  +-----------------+

As you can see, Blowfish encryption is the most memory-hungry feature.
So if you have no use for it, you can save a page and a half.

If you decide that your floppy collection is complete for now, you can
remove the collection manager and the trash manager. This means that you
will not be able to rename a floppy since it would require rearranging
the collection.

If you are satisfied with the current configuration and have no need for
the catalog function, you can also remove page 4.

You can reclaim the lower part of page 1 if you don't use RAMBO mode,
nor call the `CLOCK `DSR (time stamping still works though). Similarly,
if you don't use `LD `nor the autostart feature (or only to launch
programs that set up their own character patterns) you can reclaim the
lower part of page 2.

Finally, you can decrease the number of opened files, so as to use less
pages to buffer them. Opened files buffers are always in the pages
immediately following the system. There is room for 5 files per page.

Once you have made up your mind as to what you want to keep, you can
remove some of the `IDEAL/x` files. Each time you remove such a file,
replace it with a copy of the file called `NOLOAD` renamed as `IDEAL/x`.
Here is the list of system files:

    IDEAL/A  Page 0 high (required)
    IDEAL/B  Page 1 low
    IDEAL/C  Page 1 high (required)
    IDEAL/D  Page 2 low
    IDEAL/E  Page 2 high (required)
    IDEAL/F  Page 3 high
    IDEAL/G  Gram Card-specific routines (in page 2)
    IDEAL/H  Page 4 low
    IDEAL/I  Page 4 high
    IDEAL/J  Page 5 high (page 5 low is filled on the fly)
    IDEAL/K  Page 6 high
    IDEAL/L  Buffer pages (required)
    IDEAL/M  Page 0 low (required, must be last file)

To avoid crashes, you should edit your system configuration and indicate
which features have been disabled (record 20 in `IDE.CONFIG`).
Obviously, if you disable the configuration feature, you should do this
before removing the file!

If you ever need the full-fledge IDEAL, you can always load it from a
floppy with `IDELOAD`.

#### Non-standard IDE card

If you did not build the IDE card according to specs, it is still
possible to use IDEAL but you must configure it correspondingly with the
hardware part of `IDE.CONFIG` (records 20 and 21, see
[below](#Configuration)). The problem here is that, if you used
different CRU bits than me, `IDE.CONFIG` won't work. Thus you must patch
`IDELOAD `so that it places the proper CRU values in IDEAL and in the
bootstrap.

To this end, edit `IDE-LD2/O` with a text editor. It's a non-compressed
DF80 file, with all the parameters needed by `IDELOAD`. Modify them
according to your hardware. The file is commented for your convenience.

Remember that CRU bits are off at power-up time, so any function that
you want to be the default should be active-low. For active-low bits,
enter the string:

    9yyyyB1E0xB1D0xF

Where x is the bit number, \>1D0x is the code for `SBO `and \>1E0x is
the opcode for `SBZ`. (9yyyy is the loading address, leave it
unchanged).

For active-high bits, just invert the two commands:

    9yyyyB1D0xB1E0xF

If you did not implement a bit, replace both commands with NOPs (opcode
\>1000):

    9yyyyB1000B1000F

Once done, save the file back in DF80 format. If your text editor won't
let you do this (e.g. Funnelweb's), save it as DF80 and use the TI-Basic
program VAR2FIX to convert it to DF80.

Alternatively, you can edit the source file `IDE-LD2/S` and assemble it.

In any case, once you have set up the proper bit values, the procedure
is the following:

- Load `IDELOAD `with Editor/Assembler option 5 (or equivalent).
- Press \ to exit immediately.
- Load `IDE-LD2/O` with Editor/Assembler option3 (or equivalent). It
  will auto-start and take you to the main menu in `IDELOAD`.
- Proceed as above to load your system.

Yeah, I know it's complicated. That's what you get for not following the
specs...

##  Overview

#### File access

All virtual floppies in your collection can be accessed by name with the
**DSK@** device. For instance:

`OLD DSK@.DISKNAME.FILENAME`

Note the similarity with the `DSK.DISKNAME.FILENAME` syntax used by the
floppy disk controller. In fact, the latter will also work with the IDE
card, but only if its CRU address is set as \>1000. This is because the
floppy controller, which is the second card scanned (its CRU is \>1100),
returns an error if it does not find `DISKNAME `in one of its drives. Of
course, if you don't have a floppy controller installed, there won't be
any problem with using `DSK.DISKNAME.FILENAME`, no matter what the CRU
of the IDE card is.

Alternatively, a virtual floppy can be "inserted" into a virtual drive.
There are eight such drives, with names in the format **DSKn**, where n
is a number from 1 to 9, or a letter from A to Z. Once a virtual floppy
is inserted, it can be accessed as if it were in a real drive:

    OPEN #1:"DSK9.FILENAME",INPUT

A difference with the standard TI floppy controller is that, if the file
already exists, you don't need to specify its type when opening it: if
the record size is 0 (which it will be in Basic if you do not specify a
record length), the size and file type are supplied from disk. Note
however that Basic requires a comma after the filename in `OPEN`, so
you'll need to supply at least one parameter (like `INPUT `in the
example above).

Another difference is that you can now open fixed files in append mode.

Inserting a floppy in a virtual drive is the only way you can make it
available to the standard sector-access subprograms (subprograms \>10 to
\>15). As most disk managers exclusively use these subprograms (e.g.
DISKU), this feature will come handy with them.

####  Catalog access

You can catalog the floppy currently in a virtual drive just the way you
would with a real floppy:

`OPEN #1:"DSK9.",INTERNAL, FIXED 38, INPUT`

or, using the disk name:

`OPEN #1: "DSK@.DISKNAME.", INTERNAL, FIXED 38, INPUT`

In addition, IDEAL 1.0 offers a more sophisticated catalog access, see
[below](#Catalog).

####   Inserting a virtual floppy into a virtual drive

To "insert" a virtual floppy into a virtual drive use the **CD**
subprogram. It can be used in several ways:

1\) With the disk name:

    CALL CD.9..DISKNAME       or
    DELETE "CD.9..DISKNAME"

The double-dot syntax tells the system that you are not trying to access
a file called `DISKNAME `on the floppy currently in `DSK9`, but rather
to insert the floppy called `DISKNAME `into the drive `DSK9`.

Note that CD can be considered both as a subprogram, called with
`CALL `from Basic and `DSRLNK `offset \>000A from assembly, and as a
device, accessed with any file operation in Basic or offest \>0008 in
assembly:

The advantage of the later is that you can use string variables:
`DELETE A$` is legal in Basic, whereas `CALL A$` is not. Of course,
that's not a problem when working from assembly. Note that any file
operation will do, `DELETE `is just convenient because it does not take
any file number in Basic. And don't worry, nothing will be deleted: no
other action than inserting the floppy will be taken whatever file
operation you use.

2\) By following a directory path:

If the virtual floppy currently in the virtual drive contains a
[directory pointer file](#Subdirectories), you can follow it to access
the corresponding disk. This will automatically insert the floppy the
directory file is pointing at, in place of the current floppy. For
instance:

    CALL CD.9.MYDIR         or
    DELETE "CD.9.MYDIR"     or
    OPEN #1:"DSK9.MYDIR",INTERNAL,FIXED 1,INPUT

will insert into `DSK9 `the floppy pointed at by the file `MYDIR,`
currently in `DSK9`.

3\) Finally, how to insert a floppy into a virtual drive when working
within a disk manager program, like DISKU?

The problem here is that DISKU does almost everything with the
sector-access subprogram \>10, which does not take any filename.
Exceptions are Viewing and Deleting files. So here is the technique: 1)
Create a file with the name of the disk you want to insert, preceded
with a dot (e.g. `".MYDISK"`). 2) Delete it. It won't be deleted (to do
this you must rename it first), but the drive will now contain the
required disk.

Alternatively, you can just follow a directory path by attempting to
View the corresponding directory file. DISKU will `OPEN `it, which
automatically inserts the target disk into the drive. You'll get an
error message because DISKU tries to `READ`, then `CLOSE `the file,
which cannot be found any more since the disk just changed. Acknowledge
the error and press \ to update the listing: you should see the
content of the new disk.

####  Removing a virtual floppy from a virtual drive

Do just as above, but with no diskname:

    CALL CD.Z..  or
    CALL CD.Z.   or even
    CALL CD.Z

This will not delete anything: the virtual floppy remains in your
collection. It just means that it cannot be accessed via `DSKZ `any
longer.

The DSR can be used in the same manner:

    DELETE "CD.9.."  or
    DELETE "CD.9."   or
    DELETE "CD.9"

Caution: if you have opened files in this drive, you must close them
BEFORE removing the floppy, otherwise the most recent changes may be
lost (of course, this may be precisely what you are trying to do).

####  Adding a new virtual floppy to your collection

There are several methods:

1\) Use a disk manager to format a virtual drive: an empty virtual
floppy with the required size and name will be added to your collection.
If there was a floppy in this drive before, it is simply returned to the
collection; it won't be overwritten by the formatting operation.

2\) Edit your collection by opening the file `IDE.FLOPPIES `in append
mode, and create a new record containing the new disk name and its size
in sectors:

`100 OPEN #1:"IDE.FLOPPIES", INTERNAL, VARIABLE 80, APPEND`
`110 PRINT #1:"DISKNAME",360`
`120 CLOSE #1`

####  Removing a virtual floppy from your collection

Edit your collection by opening the file `IDE.FLOPPIES` in update mode,
locate the record of interest and overwrite it with an empty name and a
size of -1:

`100 OPEN #1:"IDE.FLOPPIES", INTERNAL, VARIABLE 80, APPEND`
`110 I=1`
`120 INPUT #1, REC I:A$`
`130 IF A$="DISKNAME" THEN 200`
`140 I=I+1`
`150 GOTO 120`
`200 PRINT #1, REC I:"",-1`
`210 CLOSE #1`

NB. There is a faster way to locate the proper record, see
[below](#Collection). Also, there is a way to recover floppies deleted
by mistake.

##  Subdirectories

IDEAL 1.0 provides a rudimentary implementation of subdirectories: it
lets you define files that do not contain any data, but rather point at
another virtual floppy. This second floppy can thus be considered as a
subdirectory of the first one. Note that, unlike PC folders,
subdirectories don't need to be arranged in a tree: a given floppy can
be a subdirectory of many others (there can even be circular references,
so be carefull). In fact, directories are more like Windows shortcuts,
except that they point to virtual floppies, not to files.

To a regular disk manager, subdirectories appear as empty Int/Fix 1 or
Int/Var 1 files. Only IDEAL recognizes them as directory pointers and
uses them accordingly.

#### Accessing subdirectories

To access a file in a subdirectory of the floppy currently in a virtual
drive, use the following syntax:

`OPEN #1: "DSK7.DIRECTORY.FILENAME"`

Where `FILENAME `is a file located in the virtual floppy pointed at by
the pointer file named `DIRECTORY`. To make things clearer, I'd suggest
that you always give your pointer files the same name as the floppy they
are pointing at. In fact IDEAL can do this automatically for Int/Fix 1
directory files: whenever you rename a floppy, all the files pointing to
it are also renamed (unless a file with this name already exists, in
which case the pointer file becomes Int/Var 1). When you remove the
floppy from your collection, all pointer files leading to it are deleted
or, if protected, marked as invalid (Int/Fix 2 or Int/Var 2).

If the target floppy itself contains pointer files, you can specify a
whole directory path:

`SAVE DSK6.DIR.SUBDIR.SUBSUBDIR.FILENAME`
When you create a pointer file pointing to a floppy, this floppy
remember the current directory as its "parent" (if you create more than
one pointer to a given floppy, you can choose which one will be the
parent). You can walk back to the parent directory by using "\<" as a
subdirectory name (which means that "\<" is not a valid diskname in your
collection):

`OLD DSKA.<.FILENAME`

This looks for the file "filename" in the parent of the disk currently
in drive `DSKA, `i.e. a disk that contains a pointer file pointing at
the current disk.

And of course, you can combine all these in fairly cryptic pathes like
this one:

`DELETE "DSK@.DISKNAME.<.<.DIRECTORY.SUBDIR.FILENAME"`

Let's see; this will delete the file `FILENAME`, in the virtual floppy
pointed at by `SUBDIR`, which is a pointer file located on a floppy
pointed at by `DIRECTORY`, which itself is located on the grand-parent
of the floppy named `DISKNAME`. I'd be quite careful before I start
using such a complicated path though...

####  Changing directories

To insert the floppy corresponding to a pointer file located in the
current drive you can use the subprogram CD:

    CALL CD.7.DIRECTORY

Alternatively, you could use DELETE in the same way:

    DELETE "CD.7.DIRECTORY"

Both will insert the floppy pointed at by `DIRECTORY `into `DSK7 (`and
`n`othing will be deleted). The file `DIRECTORY `must be present in the
virtual floppy currently inserted in `DSK7`.

Alternatively, you have the option of specifying the name of the floppy
to be inserted:

    CALL CD.8..DISKNAME or
    DELETE "CD.8..DISKNAME"

Note the double-dot syntax, it indicates that `DISKNAME `is a virtual
floppy, not a pointer file.

Finally, you can make combinations:

    CALL CD.B..DISKNAME.<.DIRECTORY.SUBDIR

In the above example, \< indicates the parent directory of `DISKNAME`,
i.e. a virtual floppy that contains a pointer file pointing at
`DISKNAME`. On this parent disk should be another file called
`DIRECTORY `pointing at another disk. On that other disk should be a
file called `SUBDIR `pointing at yet another disk, which is the one that
will ultimately be inserted. You don't have to make it so complicated
though...

####  Deleting directories

You should always use the official `DELETE `operation (opcode \>07) to
delete a directory file:

    DELETE "DSKG.DIRFILE"

Don't worry, this will not delete the floppy pointed at, only the file
that points at it.

Never attempt to delete pointer files "manually" with sector edition:
only the `DELETE `opcode knows how to maintain proper linking between
all directory files that point at the same disk. Beware of disk managers
that may try to do it in this waym rather than calling opcode \>07.

####  Creating directories

To create a new directory pointer, just create a new Int/Fix 1 or
Int/Var 1 file, with the following syntax:

    OPEN #1:"DSK5.FILENAME..DISKNAME", INTERNAL, FIXED 1, OUTPUT

Note the double-dot syntax: it tells IDEAL that you are not really
trying to create a regular IF1 file, but rather a pointer file that
points to the target floppy named `DISKNAME`. If you chose to make it a
Fixed file, IDEAL will automatically substitute `DISKNAME `for the
filename (which may cause an error if a file with this name already
exists).

It is not allowed to overwrite an existing directory, even to point it
at the very same disk. If the file you are creating already exists, you
will get a write protection error (code 1), even if the file was not
protected. To modify a pointer file, you must first delete it, then
create it anew.

Opening the pointer file as `OUTPUT `will cause the disk it resides on
to become the parent directory of the target disk. If you open the file
as `UPDATE `(the default), and the target disk already has a parent, it
will keep it. Note that you can set an [option](#user%20options) while
configuring IDEAL that forbids you to change the parent directory. When
this flag is set, a given disk can only have one Int/Fix 1 directory
file pointing at it: its parent. This forces you to arrange your disks
in a tree-wise manner, as on a PC, although no provision is made to
avoid circular references. You can define extra Int/Var pointer files
pointing at this disk, but only if automatic update has been disabled
for Int/Var files (see below).

Finally, using `SAVE `instead of `OPEN `will also work to create a
pointer file, but only `OPEN `lets you specify file options: if you use
`SAVE `the default type is always Int/Fix 1, and the parent directory
will not be changed if it already exists.

####  Dynamic directory update

Int/Fix 1 pointer files are automatically renamed when you change the
name of the floppy they point at. That is, provided there isn't already
a file with this name on the disk they reside on, in which case the file
type would be changed to Int/Var 1. Int/Var 1 pointer files are never
renamed.

If you remove the target floppy from your floppy collection, all pointer
files pointing at it will be deleted. If a pointer file is protected and
cannot be deleted, it is changed into a Int/Fix 2 file and unprotected.
This feature is mandatory for Int/Fix pointer files and optional for
Int/Var files: these may be automatically deleted or not, depending on
an [option](#user%20options) flag that you set when configuring IDEAL.

CAUTION: Dynamic update relies on the fact that a given pointer file
will always be at the same location on your hard drive. Therefore, do
not copy a pointer file from a disk to another: it could not be properly
updated. Instead, create another pointer on the second disk, using
`OPEN `or `SAVE `as described above. For the same reason, do not use a
disk manager that rearranges files on disk when making your safety
backup: use a sector-copy program instead. Always use `DELETE `to delete
pointer files (rather than poking sectors 0 and 1) and never, ever
doctor the information you will find at bytes \>20-2D in the FDR of a
pointer file.

This becomes even more of a problem when a pointer file is located on an
encrypted floppy and you did not turn encryption on. This pointer file
won't be updated but, which is worse, it cannot even be read and
prevents IDEAL from walking the pointer chain any further. One way to
avoid this problem is to always turn encryption on before
renaming/deleting disks and/or pointer files. Another is not to place on
an encrypted disk any pointer file pointing to a non-encrypted disk .

##  Encryption

IDEAL 1.0 gives you the option to encrypt a floppy, or a file so that
only you can access it. Encryption is performed according to Blowfish, a
very secure algorithm that has never been cracked so far. The price to
pay is a slightly slower disk access.

Any attempt to access encrypted files or floppies without enabling
Blowfish deciphering results in a file error. Using an incorrect
password will return gibberish and may even crash your program.
Furthermore, if you write something with an incorrect password, you'll
never be able to read it again! So my advise is: just after you enter
your password, make an attempt to read something that you know for sure
was encoded with this password. If you can't read it, you made a typo.

#### Entering your password

Your password only needs to be entered once. It then remains active
until either you erase it or you turn the PE-box off. Caution: reseting
the TI-99/4A will not erase your password unless you set the appropriate
[option](#user%20options) while configuring IDEAL. Also, if you have a
battery-backed SRAM on your IDE card, turning the power off leaves the
encryption key intact in memory. (In such a case you should actively
erase the key once you are done).

    DELETE "IDE.BLOWON.PASSWORD"    or
    CALL IDE.BLOWON.PASSWORD

Any file operation will do, but `DELETE `is quite convenient because it
does not affect the file buffers in Basic. A `CALL `can also be used,
but the advantage of a file operation is that you can pass a string
variable: `DELETE A$.`

`PASSWORD `should be a string of upto 56 characters.

The Blowfish routine will then create the encryption key from the number
Pi and your password, this takes about 3 seconds. From this point on,
Blowfish encryption is active. Any file or directory that was encrypted
will be automatically decrypted. Any new file (or virtual floppy) that
you create will be encrypted with the current password.

If at any time you want to change passwords, just repeate the above
operation. Caution: make sure than any opened file has been closed
before you change password or erase your key. Otherwise, any data still
in memory cannot be saved back on disk.

Note that, for proper security, your password should never be spelled
out in a Basic program as in the examples above (these assume
interactive keyboard entry). If you want to enable Blowfish encryption
from a program, use something like:

    100 PRINT "ENTER YOUR PASSWORD"
    110 INPUT PW$
    120 DELETE "IDE.BLOWON."&PW$
    130 PW$=SEG$("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
        XXXXXXXXXXXXXXXXXXXXX",1,LEN(PW$))

Line 130 serves to overwrite your password in the Basic string space, so
that a collegue cannot retrieve your password by peaking the VDP memory
while you went for a cup of coffee. It is not necessary if you reboot
your computer once you are done.

#### Turning Blowfish on/off

You may turn encryption/decryption off without erasing your password by
calling

    DELETE "IDE.BLOWOFF"    or
    CALL IDE.BLOWOFF

Alternatively, you can turn encryption off, but leave decryption on
(i.e. newly created files and floppies won't be encrypted, but
previously encrypted files will be accessed in an encrypted fashion):

    DELETE "IDE.BLOWOUT"    or
    CALL IDE.BLOWOUT

You can turn full encryption back on with:

    DELETE "IDE.BLOWON"    or
    CALL IDE.BLOWON

Again, make sure all opened files are closed before you turn encryption
off.

Note that the password can also be passed with `BLOWOUT `and `BLOWOFF`,
in the same way as with `BLOWON`.

####  Erasing the key

Your password remains in effect even if you reboot the TI-99/4A (as long
as you don't turn off the PE-Box). This was done so you can leave
encryption on while changing cartridges, etc. If that's not what you
want, you have the [option](#user%20options) to configure the system so
that it automatically wipes off the encryption key upon power-up.

For proper security, it is critical that you remove the key from memory
once you are done with accessing encrypted data. In addition, IDEAL will
also wipe out any unciphered data that may remain in the opened file
buffers. This is especially critical if you are using a battery-backed
SRAM that will retain buffer contents and encryption key even after
power was turned off. To erase the current key, just perform any file
operation on `IDE.BLOWAWAY`:

    DELETE "IDE.BLOWAWAY"    or
    CALL IDE.BLOWAWAY

####  Encrypting data

You can also access the encryption engine directly so as to
encode/decode data yourself. This is done via two pseudofiles,
`IDE.BLOWPLAIN` and `IDE.BLOWCIPHER`. As you might have guessed,
`BLOWPLAIN `is used to pass/retrieve plain text, whereas
`BLOWCIPHER `accesses the chiphertext.

`IDE.BLOWPLAIN` can be a file of any type you want, as long as the
maximum record length is 248 or less. Larger records result in a
256-byte ciphered string, which is illegal in Basic (It's ok if you work
from assembly, but be aware that a 256-byte string has a size byte of
\>00). `IDE.BLOWCIPHER` should be an `INTERNAL, FIXED 249` file. Smaller
record lengths may be ok, as long as they can accomodate all data passed
to `BLOWPLAIN `rounded up to the next multiple of 8, plus 1 byte for
string size.

The password should be specified with `IDE.BLOWOFF.`*`password`* as
explained above. That is, assuming that you don't want to turn on
encryption for your hard-drive. Otherwise, you may just as well use
`BLOWON `to set the password.

To encode data, write them to `BLOWPLAIN `and retrieve the coded version
from `BLOWCIPHER`. This will be a single string, upto 248 characters in
length. NB: It is critical that BOTH files are opened before you write
anything to one of them (otherwise you'll get an end-of-file error).

    DELETE "IDE.BLOWOFF.THIS-IS-MY-PASSWORD"
    RUN

    100 OPEN #1:"IDE.BLOWPLAIN",INTERNAL,VARIABLE 80
    110 OPEN #2:"IDE.BLOWCIPHER",INTERNAL,VARIABLE 249
    120 PRINT #1:"This is a test",1234,Y,B$
    130 INPUT #2:CODED$

To decode previously coded data, perform the opposite operation: write
the ciphered string to `BLOWCIPHER `and read back the plain data from
`BLOWPLAIN`.

    140 PRINT #2:CODED$
    150 INPUT #1:A$,X,Y,B$

Note that, because Blowfish always operates on 8-byte values, it may
have padded the end of the record with spaces. You shouldn't notice it
in Basic (that trims trailing spaces), but be aware of it when working
from assembly.

If you have battery-backed SRAMs, always close `BLOWPLAIN `when done:
this will wipe out any plain text that may remain in the card buffers.

#### What's encrypted?

On a crypted disk, all sectors are encrypted except for sectors 0 and 1.
Such a disk is created if encryption is on when you format it. A flag is
sector 0 indicates that the disk is encrypted. Be aware that individual
files cannot be super-encrypted on a disk that's already crypted.

If you encrypt a file on a non-crypted disk, a flag bit is set in its
FDRs to indicates that the file is encrypted (the FDR is a sector that
contains all the file information). The FDR itself is not encrypted,
only the sectors that make up the file. These sectors are completely
encrypted, irrespective of whether the data occupies the full sector or
if there are "junk" bytes in the end.

Since they consists of only a FDR, directory pointers are never
encrypted (unless they are on an encrypted disk, of course).

##   Extended catalog access

As mentionned above, you can obtain the catalog of a virtual floppy in
the same way as with the TI floppy disk controller:

`OPEN #1:"DSK9.",INTERNAL, FIXED 38, INPUT`

Record 0 returns disk informations, the following records return
information on each file, in alphabetical order. The first empty record
returns an empty string and 0 values. Attempts to access records beyond
this one result in an end-of-file error.

#### Extra information

IDEAL 1.0 provides you with a more sophisticated catalog access. To use
it, open the file as either:

`OPEN #1:"DSK9.",INTERNAL, VARIABLE 128`

or, from assembly language

`OPEN #1:"DSK9.",INTERNAL, VARIABLE 120`

The second form returns all numbers in the form of integers, instead of
floating point numbers. These are more convenient for assembly language
programs. Strings are still prefaced with size bytes, though.

Technical note: Actually, you can open the catalog as a file of any
type. For instance, if you want to access it as a relative file, Basic
will force you to open it as a `FIXED `file. Any file type is accepted,
only the record length matters: 38 opens the regular type catalog, 39 to
120 opens the extended catalog and passes numbers as integers, 121 to
128 passes them as floating point numbers. Finally, a record size of 129
or higher is used for sector-wise access ([see
below](#Sector%20access)). If you don't specify any record size, it
defaults to 128.

When opened as "extended", the catalog returns many additional variables
(although you don't have to input all of them, if you don't need them
all).

Record 0 returns disk information:

`INPUT #1, REC 0:NAME$,FILES,SECTORS,FREE,TIMEC,DATEC,TIMEM,DATEM,FLAGS,RES`

`NAME$`: A 10-character string containing the disk name

`FILES`: Number of files in the catalog

`SECTORS`: Number of sectors on disk

`FREE`: Number of free sectors on disk.

`TIMEC`: Time when the disk was created

`DATEC`: Date when the disk was created (0 = not specified)

`TIMEM`: Time of the last modification to the catalog

`DATEM`: Date of the last modification to the catalog (0 = not
specified)

`FLAGS`: 1 = Disk is protected, 2 = Disk is encrypted, 4 = 720 sectors,
8 = 1440 sectors, 16 = Archive bit (reset when modified).

`RES`: Reserved.

The next records return information on the files, in the order they
appear in the catalog (which should be alphabetical):

`INPUT #1:NAME$,TYPE,SECTORS,RECLEN,TIMEC,DATEC,TIMEM,DATEM,FLAGS,FDR`

`NAME$`: A 10-character string containing the file name

`TYPE`: File type: 1=Dis/Fix 2=Dis/Var 3=Int/Fix 4=Int/Var 5=program.
Negative if protected.

`SECTORS`: Number of sectors in file (including the FDR)

`RECLEN`: Record length. For program files: \# of bytes used in last
sector.

`TIMEC`: Time when the file was created

`DATEC`: Date when the file was created (0 = not specified)

`TIMEM`: Time of the last modification to the file

`DATEM`: Date of the last modification to the file (0 = not specified)

`FLAGS`: 1 = File is protected, 2 = File is encrypted, 4 = File is
fractured (i.e. uses non-adjacent sectors), 8 = Directory, 16 = Archive
bit (reset when modified).

`FDR`: Sector \# of the file descriptor (the FDR, i.e. the sector
containing all the info about the file).

The time stamps have the following form:

`DATE = (YEAR*512) + (MONTH * 32) + DAYOFMONTH`

`TIME = (HOURS * 2048) + (MINUTES *32 ) + (SECONDS / 2)`

Where `YEAR `= 0 to 99, `MONTH `= 1 to 12, `DAYOFMONTH `= 1 to 31
(whatever month it is), `HOURS `= 0 to 23, `MINUTES `= 0 to 59, and
`SECONDS `= 0 to 58.

Note that note that, in order to pack the time in a single word, it is
specified in 2-second increments.

A `TIMEM `value of 65535 is understood as a command for IDEAL not to
update the time stamps for this file/floppy. You also have the
[option](#user%20options) to globally disable stamping of all floppies
and/or all files when configuring your system.

####  Writing to the catalog

You can perform a number of disk operations without a disk manager, by
just writing to the appropriate record. Any field that you want to leave
unchanged should contain zero.

`NAME$`: you can change the name of a file but you cannot rename the
disk (for this use IDE.FLOPPIES, see below). Caution, the name should be
unique: if another file with the same name already exist a "file error"
is issued. Leave this string empty to keep the same name.

`SECTORS`: You can decrease or increase the number of sectors on disk.
However, you should not exceed the number of sectors indicated in the
`FLAG `word (this is the maximum space reserved on the hard disk for
this floppy).

`DATEx` and `TIMEx`: You can always change a date and time, even that of
creation. If you set the date of last modification to 65535 (\>FFFF),
neither the date nor the time will be updated when the file is modified.
If you set a value a zero, the current value, as read from the clock,
will be used instead. This works for both the disk and the files.

`TYPE `and `RECLEN`: You can change the file type and record length, but
be carefull: this may well render the file unusable. Negative
`TYPE `values result in protecting the file, positive values do not
affect the protection status unless the value is out of range (e.g.
`TYPE `= 99) although it is probably more convenient to use the
`FLAG `bits to change the file protection. Here are some examples of
modifications:

Program ---\> DF128 (or DF64 or DF32, etc). Very convenient to patch a
program file from Basic...

Any --\> Program. A program file can contain any type of data. Note that
`RECLEN `will now correspond to the end-of-file offset (last byte in
last sector).

DF16 --\> DF32 Expands a fixed file by merging its records. Safe as long
as it does not result in a record not fitting in the sector.

DF16 --\>DF48 won't work: there are sixteen DF16 records per sector, but
five DF48 records. These can only accomodate fifteen DF16 records!

DF80 --\> DF40 It is generally ok to split fixed records in several
smaller records, provided this does not result in using the unused bytes
at the end of the sector.

DF200 --\>DF50 Here is a case when it's not ok: there are 56 unused
bytes per sector with DF200 files. If you make it DF50, there will be
five DF50 records per sector, the last one containing only garbage.

DV80 --\> DV130 It's always ok to extend the maximum record size, since
no record should be greater than the current size anyhow.

DV80 --\> DV60 This may result in loss of data, unless you know for sure
that no record on this file is greater than 60.

`FLAGS`: You can modify the following flags: Disk protected, File
protected, Archive bit for either disk or file.

#### Archive bit

Each time a file is modified and its FDR (File Descriptor Record, a
sector that contains the file name and all kind of info about it) is
written back to the disk, a special "archive" bit is reset in the FDR.
This allows automatic backup programs to monitor which file has changed.
All such a program needs to do is to set the archive bit in the FDR of
any file it backs up. Next time you do a backup, the program will only
archive files whose "archive" bit was reset. Another possibility is to
compare the "last modification date" of the file with that of the
archive, and back up the file if it's greater.

The disk iteself also contains an archive bit which is modified only if
the disk catalog is modified. Which means that if you write to a file,
the directory archive bit won't necessarily be changed (it may if you
changed the file size, which modifies the sector bitmap).

###  Sector access

Provision is made for you to access a virtual floppy in a sector-wise
manner. But be carefull: this is an extremely powerfull feature and you
can hopelessly mess up a virtual floppy if you don't know what you're
doing. In fact, you have the [option](#user%20options) to turn this
feature off while configuring IDEAL 1.0, so that no malicious program
will be able to damage your virtual floppies.

Note: From assembly language, it is more convenient to use subprogram
\>10 to perform sector access.

To access a floppy sector-wise, open its catalog as an
`INTERNAL, FIXED 129` file. Each sector maps to two records in the file,
in numeric order. Each record contains a single 128-byte string that
corresponds to half a sector.

`100 OPEN #1: "DSKV.",INTERNAL,FIXED 129,RELATIVE`
`110 INPUT #1, REC 2: SEC1A$`
`120 INPUT #1, REC 3: SEC1B$`

Sector numbering starts from zero, so records 0 and 1 correspond to
sector 0, records 2 and 3 to sector 1, etc. The above program reads the
content of sector 1 into the strings `SEC1A$` that contains the first
128 bytes in the sector, and `SEC1B$` that contains the last 128 bytes.

You can modify these strings at will, then write them back at the same,
or another location. If the string you write is smaller than 128
characters, only that many bytes are modified and the end of the
half-sector remains unchanged.

The file can also be opened as a `SEQUENTIAL `file, but don't forget
that each read/write operation moves to the next record. So if you
`PRINT `back a string just after you `INPUT `it, you will actually write
it to the next record!

Technical notes:

You may have noticed that all bytes at the end of sector 0 in each
vitual floppy (bytes 238-255) read as 255, no matter what you write
there. IDEAL uses these bytes to store the information needed to manage
the floppy collection: links to the previous and next floppy, to the
parent directory, flag bits, etc. Altering these bytes may result in
hopelessly messing up your floppy collection. For this reason, neither
subprogram \>10 nor the above sector-wise access technique let you
modify them. In addition, I had to camouflage them as \>FF upon reading,
because DISKU would panic when trying to write to sector 0 and reading
different data!

Also note that if you alter the disk name in sector 0 your virtual
floppy collection will be rearranged, which could take a few seconds. If
you have protected your collection, the write operation will fail with a
"write-protected" error.

##  Editing your virtual floppy collection

Your virtual floppy collection can be accessed via a pseudo-file named:
`IDE.FLOPPIES`. This file must be opened as `INTERNAL, FIXED 128 `from
Basic and as `INTERNAL, FIXED 120` from assembly. The difference is that
numbers are passed as floating point values in Basic and as integers in
assembly. Strings are passed with a leading size byte in both cases.

You can open the file as either `RELATIVE `or `SEQUENTIAL `mode.
Relative files are easier to work with, but they are limited to 32767
records in Basic and to \>FFFE records in assembly, so if you have more
floppies you won't be able to access all of them. A sequential file is
more cumbersome, but has no limitations on the number of floppies. Note
that `IDE.FLOPPIES` has a special mechanism to jump to any record via
record 0 (see [below](#Floppies%20rec%200)), that works with both
sequential and relative files.

`100 OPEN #1: "IDE.FLOPPIES",INTERNAL,FIXED 128, RELATIVE`

Note: You can set a security [option](#user%20options) bit while
configuring your system, so that the collection cannot be modified by
writing to `IDE.FLOPPIES`. This will prevent malicious programs from
messing with your collection. While this bit is set, you can read from
`IDE.FLOPPIES`, but not write to it. Also you cannot use a disk manager
to rename a virtual floppy, nor to create a new one. Trying to do so
will result in an I/O error \#5.

####  Retrieving floppy disk parameters

Record 0 has a special meaning so we'll set it aside for the moment
being. Each of the next records accesses one of your virtual floppies,
arranged in alphabetical order. Note that, because Basic file operations
limit the number of records to 32767, you won't be able to access more
virtual floppies than this, unless you use a sequential file (from
assembly, you can reach upto 65534 records).

`INPUT #1, REC X: NAME$,SECTORS,TIMEC,DATEC,TIMEM,DATEM,FLAGS,RES`

`NAME$`: A 10-character string containing the disk name

`SECTORS`: Apparent number of sectors in the disk, as stated on sector
0. The actual amount of memory reserved for the virtual floppy depends
on flag bits 4 and 8: 360, 720 or 1440 sectors. The `SECTORS `value
should be small enough to fit in there. A value of 0 defaults to this
size.

`DATEC`: Date when the disk was created (-1 = not used, 0=now)

`TIMEC`: Time when the disk was created (-1 = not used, 0=now)

`DATEM`: Date of the last modification to the catalog (-1 = not used,
0=now)

`TIMEM`: Time of the last modification to the catalog (-1 = not used,
0=now)

`FLAGS`: 1 = Protect disk, 2 = Encrypt disk, 4 = 720 sectors, 8 = 1440
sectors, 16 = Archive bit (reset when modified).

`RES`: Reserved.

`INPUT `automatically moves to the next record each time it's called so
you can easily travel a sequential file, or a relative file without a
`REC `clause, by just repetedly reading its records.

####  Modifying a floppy

You can modify any of these parameters by writing back to the same
record. The `PRINT `instruction always writes to the last record that
was read (or searched for) and does not move to the next record. This
was done so you can read records until you find the one you want and
then modify it. To write to the next record, you must first `INPUT `it,
then write to it. Of course, if you are using a relative file with a
`REC `clause, the specified record is accessed.

The record structure is the same as described above. Any parameter that
you omit or leave as 0 remains unchanged. To keep the same name, you can
pass it as an empty string.

`PRINT #1, REC 12: "",356`

This changes the apparent disk size (as it appears on its catalog, on
sector 0) to 356 sectors.

The one thing that you cannot change is the physical size of the floppy
as determined by the flag bits, i.e. the amount of space it takes on the
hard disk. If you really want to do this, you must delete the floppy and
create another with the proper size.

####  Locating a floppy

Rather than walking your floppy collection record by record to locate a
given floppy, you can use record 0 to locate it. Just write the desired
floppy name to record 0 and the next record that you access (to read or
write) will be that of the floppy.

`PRINT #1, REC 0: "MYDISK"`

If the name does not exist, a "file error" is issued. Searching for an
empty string jumps to the end of the file, so you can add new floppies
to your collection.

You can also read back the record number corresponding to the searched
floppy, from record 0. Be aware that doing so rewinds the file, so the
next record accessed will be record 1.

`INPUT #1, REC 0: A$,R `

or, for sequential files

`RESTORE #1`
`INPUT #1: A$,R`

A\$ will contain the string you passed with the `PRINT `statement above
(if any). R contains the record number that corresponds to this floppy,
or 0 if the floppy wasn't found.

N.B. None of this can be done with the trash files.

####  Creating a new floppy

To create a new floppy, move to the end of the file by passing an empty
string to record 0 or by opening the file in `APPEND `mode. Then simply
write a record with the required parameters: name, size, etc. Actually,
only the name is required: all other parameters have default values
(360,-1,-1,-1,-1,0,0). But the name must be unique: if another floppy
with the same name already exists, nothing is created and a file error
is issued.

CAUTION: every time you create, delete or rename a floppy, your
collection is rearranged in alphabetical order. Thus, you should never
assume that a given record will access the same floppy after an
addition/deletion than it did previously. In addition, as your
collection grows, you may notice a delay of a few seconds, and a lot of
disk activity while the collection is rearranged.

####  Deleting a floppy

To remove a floppy from your collection, you can use opcode \>08
(scratch record) from assembly, or just overwrite its record with an
empty name string AND a size of -1. Removed floppies are placed in a
trash file (there are three of them, one per floppy size) so the space
they occupied can be reclaimed next time you create a new floppy. Until
this happens, you have the possibility to recover deleted floppies from
the trash. You also have the [option](#user%20options) to configure
IDEAL 1.0 so that it only recycles the trashed floppies once your hard
drive is full.

####  Recovering a deleted floppy

A floppy that was erased by mistake can be recovered from the trash by
opening `IDE.TRASHn` where n is a number from "1" to "3" and is used to
access the trash files that respectively contains the 360-, 720- and
1440-sector floppies. The file should be opened in update mode, as an
Int/Fix file with a record size of 11 or more. It can be either
sequential or relative.

To recover a floppy from the trash, you must first locate it. The most
recently deleted floppy is in record 0, the oldest one at the end of the
file: this is the one that will be recycled next. Each record consists
in only one string of at most 10 characters, which contains the name of
the deleted floppy.

Once you found the record containing your floppy, just write a new name
(or the same one) to its record. This will automatically return the
floppy in your collection, provided the disk name does not already exist
in the collection, in which case a file error would be issued.

    90 REM Recover a 360-sector floppy named HERE-IT-IS
    100 OPEN #1:"IDE.TRASH1", INTERNAL, FIXED 80, UPDATE, RELATIVE
    110 I=-1
    120 I=1+1
    130 INPUT #1,REC I:NAME$
    140 IF NAME$<>"HERE-IT-IS" THEN 120
    150 REM we found it in record I. Let's recover it (with same name)
    160 PRINT #1,REC I:NAME$
    170 REM we could use ON ERROR in case the name already exists, to pick a different name
    180 CLOSE #1

You can also open the file as sequential: every read operation moves
forward to the next record. A write operation accesses the record that
was read last (or record 0 if the file was just open).

    90 REM Recover a 720-sector floppy named THAT-ONE
    100 OPEN #1:"IDE.TRASH2", INTERNAL, FIXED 22, UPDATE, SEQUENTIAL
    110 RESTORE #1
    120 INPUT #1:NAME$
    130 IF NAME$<>"THAT-ONE" THEN 120
    130 REM we found it. Let's recover it with a different name.
    140 PRINT #1:"NEWNAME"
    150 CLOSE #1

##  Clock access

You can access the real-time clock onboard the IDE card by opening a
pseudo-file called CLOCK (by default, but you can change this name while
configuring your system). It should be opened as internal, fixed 128 in
relative mode. You can then access the various clock functions through
specific records.

    100 OPEN #1:"CLOCK", INTERNAL, FIXED 128, RELATIVE

Most variables can be modified by just writing the new values to the
appropriate record. Note that you must write all elements in a record,
even if you leave some unchanged. Illegal values are ignored, so it is a
convenient way to leave an element unchanged (alternatively, read it
first and write back what you read).

There is no need to close the file when done, but it won't hurt to do
it:

    500 CLOSE #1

#### Record 0: Date

    110 INPUT #1,REC 0:WEEKDAY,DAY,MONTH,YEAR1,YEAR2

`WEEKDAY `is the day-of-the-week. Sunday is 1, Monday is 2, etc.

`DAY `is the day-of-the-month

`MONTH `is the month

`YEAR1 `contains the first two digits of the year (e.g. 19 or 20).

`YEAR2 `contains the last two digits of the year. For a 4-digit date
calculate `(100*YEAR1)+YEAR2`.

#### Record 1: Time

    120 INPUT #1, REC 1: HOURS,MINUTES,SECONDS,SETTINGS

`HOURS `contains the hour. In 24-hour mode, valid values are 0
(midnight) to 23 (11 pm). In am/pm mode, valid values are 1 to 12 (am)
and 129 to 140 (pm). As you can see, pm hours are marked by adding 128
to the hour. When you set the time, make sure that the convention you
are using is the one specified in `SETTINGS`, otherwise you may end up
with crazy values (e.g. 17 am).

`MINUTES `Valid values are 0 to 59.

`SECONDS `Valid values are 0 to 59.

`SETTINGS `are the clock settings: 2=clock is in 24-hour mode.
1=daylight savings mode enabled: the clock automatically gains an hour
at 2:00 am the first Sunday in April and looses it at 2:00 am the last
Sunday in October.

#### Record 2: Alarm

You can set one alarm that will occur every day at the same time.
Optionally, you can make it an hourly alarm or even an alarm that occurs
every minute. This is done by placing a "don't care" value in the
appropriate alarm register. To disable alarms, set a value of 60 for the
seconds.

    130 INPUT #1,REC 2:HOUR,MINUTE,SECOND,INTF,ACTION,VECT,REC,SCRAD

`HOUR `is the hour at which the alarm must ring. The format is the same
as for `HOURS `in record 1, except that values 192 to 215 are also
legal: they mean that the alarm should occur whatever hour it is (i.e.
bits \>C0 are set).

`MINUTE` Legal values are 0 to 59. Values from 192 to 251 disregard the
minutes for alarm purposes.

`SECOND `Legal values are 0 to 59. Values from 192 to 251 ignore the
seconds. To disable alarms completely just enter 60.

`INTF `adds up the interrupt flags: 16=alarm interrupt enabled, 8=an
interrupt occured, 4=it was a periodic interrupt, 2=it was an alarm,
1=there was an update (occurs every second). Only flag 16 can be written
to, the others are automatically reset when accessing record 2.

`ACTION `indicates the action to take when a periodic interrupt occurs.
0=do nothing. 128=enable drive sleeping, 1=blink screen, 2=beep,
4=display record, 8=call program.

`VECT is `the address of a BLWP vector (workspace and address) where to
branch at if the call program option is enabled.

`REC `is the record number of the string you want to display (see record
numbers below, only the first string is displayed).

`SCRAD `is the screen address where to display the time or date. Add
\>80 to add the Basic bias to every character, \>40 to prevent this from
happening. If you add neither, the charater at address `SCRAD `will be
read and, if it is \>80 or greater, the Basic bias will be added to the
string.

If your IDE card is built so, the clock can trigger TI-99/4A interrupts,
either at defined intervals or when the alarm goes off. Of course,
interrupts will only be effective if the program currently running has
enabled them (with a LIMI 2 instruction). Basic and Extended Basic do
so, but some assembly programs may not.

Reading INTF lets you know what happened, writing to it lets you specify
which kind of interrupt is allowed.

Four actions can be taken when an alarm occurs: the screen can blink 3
times (this uses the copy of VDP register 1 stored at \>83D4), a beep
can sound 3 times, the time or the date can be displayed on screen in
one of the formats specified in records 4 and above, or a call can be
made to a program of yours. Any combination of these actions is allowed.

To display the time or the date, select the format desired from those
described for the records below and use this record number for the
`REC `value. For instance, use 8 to display the time as "6:55 pm". Tip:
using records 100-227 accesses the clock RAM, which allows you to
display a preset message on screen. Just place the message (with a
leading size byte) in the appropriate clock page by writing it to the
corresponding record and puts this record number in `REC`. Be carefull
not to overwrite the bootstrap program in the first clock pages.

If you call a program it MUST be in memory when the interrupt occurs,
otherwise the system will crash mercilessly. The program is entered with
a BLWP after then interrupt service routine has returned. Once done, you
can return to the interrupted program with a simple RTWP.

In addition, IDEAL uses alarms to put a drive to sleep after a given
period of inactivity. The sleep algorithm triggers an interrupt every
minute and counts them until the specified number of minutes has
elapsed. Any disk operation resets the counter. If you are using the
alarm for any other purpose, you should clear bit 128 in the
`ACTION `word so as not to mess up the count (unless you happen to have
set an alarm every minute).

Finally, turning the console off, then back on disables all interrupts.
This was done so that the computer won't lock up if an interrupt fires
when IDEAL is not loaded to acknowledge it.

#### Record 3: Periodic events

You can cause the clock to repetedly trigger an event at defined
intervals, that range from 122 microseconds to half a second. You can
also issue a square-wave signal on the corresponding pin on the IDE
card.

    140 INPUT #1,REC3:PERIOD,INTS,ACTION,DATA

`PERIOD `is the delay between two interrupts, which is also the
frequency of the square-wave output. See the IDE page for [legal
values](ide.htm#square%20wave).

`INTS `contains the control flags: 1=square-wave enabled, 8=periodic
interrupt enabled, 9=both.

`ACTION `indicates the action to take when a periodic interrupt occurs.
0=do nothing. 128=enable drive sleeping, 1=blink screen, 2=beep, 4=call
program.

`DATA `the address of a BLWP vector (workspace and address) where to
branch at if the call program option is enabled.

The same actions as for an alarm interrupt can be specified. However,
since the maximum interval is half a second, there is no point in
blinking the screen, nor beeping when such an interrupt occurs. Most
likely you will only want to call a program at this point. Just be aware
that answering an interrupt takes time. So if you set interrupts at too
short a period, the computer will end up spending most of its time
servicing interrupts!

#### Record 4 to 7: Formatted date

The next four records contains the date formatted in different ways into
text strings. These records cannot be written to.

    150 INPUT #1,REC 4:DATE$,DAY$

`DATE$` is in the form "12/25/99"

`DAY$` is in the form "Mon"

    160 INPUT #1,REC 5:DATE$,DAY$

`DATE$` is in the form "Sep 4 96"

`DAY$` is in the form "Wed"

    170 INPUT #1,REC 6:DATE$,DAY$

`DATE$` is in the form "October 1, 2000"

`DAY$` is in the form "Friday"

    180 INPUT #1,REC 7:DATE$,YEAR$,DAY$

`DATE$` is in the form "January, the 3rd"

`YEAR$` is in the form "1976"

`DAY$` is in the form "Saturday"

#### Record 8 to 11: Formatted time

The next four records contains the time formatted in different ways into
text strings. These records cannot be written to.

    190 INPUT #1,REC 8:TIME$

`TIME$` is in the form "9:55 pm"

    200 INPUT #1,REC 9:TIME$

`TIME$` is in the form "11:23:06 am"

    210 INPUT #1,REC 10:TIME$

`TIME$` is in the form "13:45"

    220 INPUT #1,REC 11:TIME$

`TIME$` is in the form "17:25:00"

####  Record 12: RAM registers

Most clock chips contain extra register that are available to the user
as battery-backed RAM. PC computers save the so-called "CMOS setup" in
these. According to the chip, there may be a total of 64 or 128
registers (the RTC-65271 has only 64), but is all cases registers 0 to
13 map to clock functions.

You can get a dump of all clock registers by accessing record 12:

    230 INPUT #1,REC 12:R0TO63$,R64TO127$

`R0TO63$` is a 64-byte string that contains registers 0 to 63, one
character per register. See the IDE page for the meaning of registers
0-13. Note that, if the first character (the seconds) is 59, the content
of the clock registers may not be accurate because the clock may have
moved to the next minute/hour/day/etc just after the seconds register
was read.

`R64TO127$` is a 64-byte string that contains registers 64 to 127, if
they exist. If your clock chip has only 64 registers, chances are that
this string will have accessed the same registers as `R0TO63$ `(and thus
will be identical unless the seconds were incremented).

You can write to this record to place new values in the clock RAM
registers. Registers 0 to 13 are reserved for clock registers, so the
first 14 characters of `R0TO63$` will be ignored. Register 14 is used by
IDEAL to store the first two digits of the year (20), so make sure you
leave this 15th character intact. By contrast `R64TO127$` is copied
entirely, which may result in modifying the clock registers if you have
only 64 registers. For this reason, you must always pass two strings
when writing to record 12: the second string should be empty unless you
are sure you have a clock chip with more than 64 registers.

    240 A$=SEG$(R0TO63$,1,15)&CHR(67)&SEG$(R0TO63$,17,64)
    250 PRINT #1, REC 12: A$,""

This example puts the number 67 into register 15 (the 16th character in
the string) and leaves the other registers unchanged.

#### Records 100 to 227: XRAM pages

If your IDE card carries the RTC-65271 clock, you have an additional 4K
of battery-backed RAM at your disposal. This memory is part of the clock
chip and arranged as 128 pages of 32 bytes. You can access these pages
via records 100 to 227: the record number corresponds to the page number
plus 100.

    300 INPUT #1, REC PAGE+100: A$

`A$` is a 32-byte string that contains the content of page `PAGE`.

You can also write to these pages, but be carefull as IDEAL reserves the
first pages for its own use (they contain the system configuration and
the boot sequence that loads IDEAL into the card SRAM). Before writing
anything, check the fourth byte in page 0: it contains the number of the
first free page (or zero if all pages are used). Feel free to update
this byte as you reserve pages for your own use.

    310 INPUT #1, REC 100:A$
    320 PAGE=ASC(SEG$(A$,4,1)
    330 PRINT #1, REC PAGE+100:MYFIRSTPAGE$

If the string you write is shorter than 32 bytes, the remainder of the
page remains unchanged.

##  Configuring IDEAL 1.0

IDEAL 1.0 has the capability to configure itself following your
instructions. To this end, open the pseudo-file `IDE.CONGIF` as an
`INTERNAL, VARIABLE 128` file. The various records provide access to the
multiple features of IDEAL 1.0.

    100 OPEN #1:"IDE.CONFIG",INTERNAL, FIXED 128, UPDATE, RELATIVE

Alternatively, you can open it as Int/Fix 120, in which case all numbers
will be passed as integers (or 32-bit long integers for LBAs). This is
usefull when working from assembly, so you don't have to deal with
floating point numbers.

Technical note: actually, the file type is not checked, only the record
size is. A record size of 128 or above uses floating point numbers, any
size under 128 uses integers. Basic stupidely prevents you to open
variable files as relative, so you must either use `FIXED`, or open the
file as `SEQUENTIAL`. There is no such limitation if you work from
assembly.

You can modify some of these values by writing to the desired record.
Note that some records are read-only.

Once you are done, don't forget to close the file: it's at this point
that the configuration is saved on disk. When you write to the file, the
changes are only saved in the clock XRAM.

    500 CLOSE #1

By the way, saving the configuration to the hard drive is something that
you may want to do from time to time, just is case the clock battery
would run out... To this end, you must open `IDE.CONFIG`, write to it at
least once (e.g. to enter your password), the close it.

#### Record 0

    110 INPUT #1:PASSWORD$, LOCK, VERSION, PARTITION, VIRDRIVE

`PASSWORD$` is a password string that lets you write-protect IDE.CONFIG
so that a malicious program cannot fool around with your system
settings. The password can be upto 32 characters in length, by default
it is "ThN" (my initials). For obvious reasons, `PASSWORD$` is
write-only and always reads as an empty string.

`LOCK `is the lock status flag: if it is 0, you don't need a password to
write to IDE.CONFIG (except to change `PASSWORD `and `LOCK)`. When
`LOCK `is different from 0, you will need to write your password to
record 0 before you can write to any other record. NB You don't need to
change `LOCK `when doing so: the file is temporarily unlocked when you
enter your password and will be relocked once you close it.

`VERSION` Version number of IDEAL (Currently 1.0). Read only.

`PARTITION` Current partition (currently, only partition 1 is
available). Read only.

`VIRDRIVE` Current virtual drive. Read only.

To access a locked file, all you need to do is to write your password to
record 0. The file is temporarily unlocked, until you close it. If you
want to unlock it permanently, set `LOCK `as 0.

To change your password you must write twice to record 0: first write
the **old** password and set `LOCK `as 1, then write the new password
and set `LOCK `as you like.

#### Record 1 through 4

These records access the current directory in partitions 1 through 4.
Not implemented in version 1.0.

    110 INPUT #1:PATH$,DRIVENB,LBA

`PATH$` Path to the current directory in this partition. Currently, set
as partition root (i.e. A.).

`DRIVENB` Drive number through wich this partition can be accessed
(currently \>61).

`LBA `Address of the current directory in the partition.

#### Record 5

    110 INPUT #1,REC 5: SYSPATH$,MAXLBA,SLAVELBA,ROOTLBA,TOPLBA,SYSLBA

`SYSPATH$` Diskname and filename of the first system file (e.g.
"`DISKNAME.IDEAL/A`").

`MAXLBA `Last address that can be used by the floppy collection in the
master drive.

`SLAVELBA `Last address in the slave drive (if any).

`ROOTLBA `Address of the virtual floppy disk collection (floppy at root
of the tree). Read only.

`TOPLBA `Current address reached by the floppy collection (there may be
some empty slots in this space, if you deleted floppies). Read only.

`SYSLBA` Address of the virtual floppy containing the system files. Read
only.

`MAXLBA `and `SLAVELBA `let you define the size of your master and slave
drive respectively. This space will be available for the floppy disk
collection. If you wanted to reserve space for other purposes on the
drive, you could set either parameter to less than the actual disk size.
If you have no slave drive, just set `SLAVELBA `as 0, so that the floppy
collection won't be expanded on the slave drive once the master is full.

`TOPLBA` and `ROOTLBA `are internal pointers to your floppy disk
collection. They are automatically updated when you edit the collection.

With `SYSPATH$` you indicate the location of the IDEAL system files, so
they can be loaded at power-up time. It must be in the form of a
diskname, a dot, and the name of the first system file. When you write
to `SYSPATH$`, the corresponding `SYSLBA `is automatically computed or a
"file error" is issued if the disk does not exist (but the presence of
the system files on this disk is not verified).

#### Record 6 through 9

These records access the four partitions. Only partition 1 is
implemented in IDEAL 1.0.

    110 INPUT #1: NAME$,DRIVENB,LBA,SIZE

`NAME$` Name of the DSR for the partition (Currently "A" for partition
1).

`DRIVENB `Drive number to access the partition from subprograms
(Currently \>61 for partition 1).

`LBA `Address of the partition.

`SIZE `Size of the partition, in 512-byte sectors.

#### Record 10 through 17

These records access the virtual drives DSRs that emulate floppy drives.

    110 INPUT #1:NAME$,DRIVENB,FLOPPYLBA

`NAME$` Name of the DSR for this drive (Normally `DSK1 `through
`DSK9 `and `DSKA `through `DSKZ`).

`DRIVENB` Drive number to access this drive from subprograms (for
`DSKx`, ascii of x minus \>30, e.g. `DSK2` = \>02, etc).

`FLOPPYLBA` Hard disk address of the virtual floppy "inserted" in this
drive (-1 if none).

#### Record 18

    200 INPUT #1:RTCNAME$

`RTCNAME$`: Name of the clock access DSR. By default "`CLOCK`".

#### Record 19

    130 INPUT #1:FULLNAME$,ONOFF,XBFULLNAME$,CRU

`FULLNAME$`: Specifies which program is launched upon power-up (if any).
It must be in the form `DISKNAME.FILENAME` (no subdirectory path). The
file should be a "program" file, containing either an assembly language
program, an Extended Basic program (in which case the Extended Basic
cartridge should be plugged in, or loadable in a GRAM-card), or a GPL
program (such as a cartridge) to be loaded in your GRAM-card.

`ONOFF`: Determines whether the program will start automatically upon
power up. 0=no, 1=yes, 2=load but don't start.

`XBFULLNAME$`: Specifies the location of the Extended Basic cartridge to
be loaded in a GRAM-card. It must be the name of the first file, in the
form `DISKNAME.FILENAME`.

`CRU `is the base CRU of your GRAM-card, if you have one. 0 stands for
\>1000, 1 for \>1100, ... 15 for \>1F00.

The existance of `FULLNAME$` and `XBFULLNAME$` is only verified when
they are actually used, so you won't get an error if you enter the name
of a file that does not exist (yet).

#### Record 20

This record collects a whole bunch of options. To set one or more
options, add up the values of the corresponding flags and pass the
result in `OPTIONS`.

    240 INPUT #1:OPTIONS, SLEEP, OPENED, MAXOPEN

`OPTIONS`: User preferences flags. Just add up the flag values of the
desired options.

`SLEEP`: Number of minutes of inactivity before the drives go to sleep
(0=ignored).

`` OPENED` `` Number of opened files allowed simultaneously (just like
CALL FILES, but IDE-specific).

`MAXOPEN `Maximum value for `OPEN`, depends on the size of the SRAM.

Option flags:
1 = Time-stamp virtual floppies.
2 = Time-stamp files.
4 = Allow sector-wise access.
8 = Recycle deleted floppies to create new ones.
16 = Wipe encryption key upon reset.
32 = Allow only one Dis/Fix directory file per disk (the parent).
64 = Do not delete Dis/Var directory files when target disk is
deleted.
128 = Do not allow modifications to the floppy collection (i.e.
write-protect `IDE.FLOPPIES`).
256= Put inactive drives to sleep.

512 = RAMBO mode not allowed (even if supported by the hardware).
1024= Encryption not supported.
2048= Floppy collection manager and trash manager not supported.
4096= Configuration manager, extended catalog and sector-wise access not
supported.
8192= `CLOCK `DSR not supported (but time stamping still is).

Flags "1" through "128" are self explanatory and have been discussed in
the relevant sections anyhow.

By setting the "256" flag, you can cause the drives to be put to sleep
after a given number of minutes of inactivity. The number of minutes can
be set with the `SLEEP `value. The sleeping algorithm makes use of the
clock alarm to trigger an interrupt every minute, and decrease a
counter. When the counter reaches zero, the "sleep" command is sent to
the drives. Accessing any DSR or CALL on the card resets the counter to
the `SLEEP `value, thereby delaying the moment to put the drives to
sleep. Once a drive is asleep it may take several seconds to wake up, so
the next drive operation could appear hanged up, but don't worry: the
command will complete as soon as the drive is ready. N.B. Modifications
to this flag only become effective when you reset your TI-99/4A.

The flags "512" through "8192" let you disable some parts of IDEAL, so
that you can save memory or loading time, by using a truncated version
of IDEAL. I'd recommend that you use the full-fledge version, but it's
up to you.

#### Record 21

This record lets you access hardware information: it describes the
peculiarities of your card, how you wired it, what is available, etc.

    250 INPUT #1:CONFIG,SRAM,RAMBO,REGBIT,SWBIT,FIXBIT,RAMBOBIT,WPBIT,RSTBIT,IRQBIT

`CONFIG`: Hardware configuration flags.

`SRAM`: Size of the on-board SRAM.

`RAMBO`: Number of available RAMBO banks (8 K each).

`REGBIT`: CRU bit that selects register access (normally 1).

`SWBIT`: CRU bit that enables bank switching. Negative if active-low, 0
if none (normally 2).

`FIXBIT`: CRU bit that freezes the area \>4000-4FFF to bank 0. Negative
if active-low, 0 if none (normally -3).

`RAMBOBIT`: CRU bit that enable RAMBO mode. Negative if active-low, 0 if
none (normally 4).

`WPBIT`: CRU bit that write-protects the SRAM. Negative if active-low, 0
if none (normally 5).

`RSTBIT`: CRU bit that resets the drives. Negated if active-low, 0 if
none (normally 7).

`IRQBIT`: CRU bit that senses drive-issued interrupts. Normally 0.

Configuration flags:
1 = has battery-backed SRAM.
2 = has XRAM in clock.
4 = clock can send interrupts.
8 = RAMBO mode supported by hardware.

##  IDE controller configuration

It is possible to query the IDE controller chip on both the master and
the slave drive. The controller then returns a set of drive parameters
following a well-established convention. Just open `IDE.MASTER` or
`IDE.SLAVE` as an IV254 file:

    OPEN #1:"IDE.MASTER",INTERNAL,VARIABLE 254,INPUT

Then you can read (but not modify) the drive parameters via the
following records:

    INPUT #1,REC 0:CONFIG,CYL,RES1,HEADS,TRACKSIZE,SECSIZE,RES2,RES3,RES4

`CONFIG`: Configuration bits

`CYL`: Number of physical cylinders (i.e. tracks)

`RES1`: Reserved

`HEADS`: Number of heads

`TRAKCSIZE`: Number of unformatted bytes per physical track

`SECSIZE`: Number of unformatted bytes per sector

`RES2`, `RES3`, `RES4`: Reserved

Configuration bits:
2 = hard-sectored
4 = soft-sectored
8 = RLL/ARLL format
16 = head switch 15 usec delay
32 = power-down mode implemented
64 = hard drive
128 = removable drive (CD ROM)
256 = transfer rate \< 5 Mbit/sec
512 = transfer rate 5 to 10 Mbit/sec
1024 = transfer rate \> 10 Mbits/sec
2048 = rotation deviation \< 0.5% (notebook drives).

    INPUT #1,REC 1:SERIAL$,BUFTYPE,BUFSIZE,ECCSIZE

`SERIAL$`: Ascii string containing the drive's serial number.

`BUFTYPE`: Buffer type. 1=one-way, 2=bidirectional, 3=cache buffer.

`BUFSIZE`: Buffer size in bytes, divided by 512.

`ECCSIZE`: Number of ECC (error correction code) bytes transfered by
read/write-long operations.

    INPUT #1,REC2:FIRMWARE$,MODEL$

`FIRMWARE$`: Ascii string containing the version number of the
controller software.

`MODEL$`: Ascii string containing the drive model.

    INPUT #1,REC3:SECPINT,IO32,DMALBA,RES5,PIOTIME,DMATIME,RES6

`SECPINT`: Number of sectors between interrupts (multiple read/write
only).

`IO32`: Flag bit. 1= 32-bit I/O.

`DMA/LBA`: Flag bits. 256=DMA (direct memory access), 512=LBA (logical
block addressing).

`PIOTIME`: I/O cycle time. 0=600 nanoseconds, 1=380 ns, 2=240 ns, 3=180
ns.

`DMATIME`: DMA cycle time. 0=960 ns, 1=380 ns, 2=240 ns, 3=150 ns.

`RES5`, `RES6`: Reserved.

    INPUT #1,REC4:LCYL,LHEADS,LTRACKSIZE,LSECSIZE,LSECPINT,LSECNB

`LCYL`: Number of logical cylinders.

`LHEADS`: Number of logical heads.

`LTRACKSIZE`: Number of logical sectors per logical track.

`LSECSIZE`: Number of bytes per logical sector.

`LSECPTINT`: Number of sectors between two interrupts.

`LSECNB`: Total number of logical sectors addressable in LBA mode.

NB. Modern drives can mimic more primitive drives and pretend they have
a different geometry (cylinders, heads, etc) than their own. This is
called address translation and it is why IDE drives should only be
formatted in factory. This mechanism may cause the above parameters to
vary according to your drive configuration.

    INPUT #1,REC5: SDMASUP,SDMA,MDMASUP,MDMA

`SDMASUP`: Single DMA modes suported.

`SDMA`: Active single DMA mode.

`MDMASUP`: Multiple DMA modes supported.

`MDMA`: Active multiple DMA mode.

Irrelevant for our purpose: the IDE card does not support DMA.
Records 6 to 9 return each 16 values corresponding to reserved
parameters.
Records 10 and 11 return each 16 manufacturer-dependent values.
Records 12 to 17 return each 16 values corresponding to reserved
parameters.

##   Direct access to the hard drives

It is possible for you to access your hard drives directly, on a
sector-by-sector basis. Just be aware that it is an extremely powerfull
feature and that you may end up completely scrambling the contents of
your drives if you don't know what you are doing. In fact, you have the
option to disable sector-wise access when configuring IDEAL, so that no
malicious program can play havoc with your drives.

To access your drives, open the file `IDE.SECTORS` as a `INTERNAL`,
`FIXED 129 `file.

    100 OPEN #1: "IDE.SECTORS", INTERNAL, FIXED 129, RELATIVE

This is a pseudofile that contains only five records. Record 0 is used
to set the sector number, and records 1 through 4 each contain a
128-byte string corresponding to a quarter of the selected sector. Oh
yes, I forgot to mention it, but hard drives have 512-byte sectors.

    110 PRINT #1, REC 0: LBA1,LBA2
    120 INPUT #1: PART1$,PART2$,PART3$,PART4$

Then you can write the strings back. The address won't change unless you
write a new one to record 0. (In case you forgot what it was, you can
read it back from record 0).

    130 PRINT #1:PART1$,PART2$,PART3$,PART4$

If one of the strings contains less than 128 bytes, or if you don't
write back all four strings, the remaining bytes in the sector are
unchanged.

You don't have to explicitely close this file, but it won't hurt doing
it:

    200 CLOSE #1

####   LBA addressing

Nowadays, most hard drives use logical block addressing (LBA). Which
means that you can consider your drive as a huge array of 512-byte
sectors, numbered from 0 to whatever your drive size allows. Addresses 0
through 268,435,455 are on the master drive, whereas addresses
268,435,456 through 536,870,912 are on the slave drive. In hexadecimal
notations, these are 0 through \>0FFFFFFF and \>10000000 through
\>1FFFFFFF. If you do the math and multiply these numbers by 512 bytes,
you'll find out that you can access 128 megabytes per drive!

For your convenience, if you pass a negative number in LBA1 it will be
ignored and LBA2 is assumed to contain an address in the slave drive, to
which the corresponding bias will be added (thus LBA2 should be in the
range 0 to 268,435,455). If LBA1 is positive, LBA2 will be ignored.

Finally, if both LBA1 and LBA2 are negative, the address is bumped up to
the next sector. This is usefull to walk the drive sector by sector:

    140 PRINT #1, REC 0:-1,-1

When working from assembly, you can avoid dealing with floating point
numbers and use long integers (32-bit words) instead. To this end, just
open the file as `INTERNAL`, `FIXED 130`. This tells IDEAL that all LBAs
should be passed to and from record 0 as long integers.

##  DSRs

IDEAL version 1.0 implements several DSRs :

- Eight virtual drives that emulate TI drives. They can have any name
  upto 7 characters, but I'd suggest you stick with DSKx, where x is a
  digit from '1' to '9', or a letter from 'A' to 'Z'. This will make
  them fully compatible with existing TI programs.
- Four partitions that will be used for full-disk access in a future
  version. Their default names are "A", "B", "C" and "D" and they are
  meant to implement two partitions on the master drive and two on the
  slave drive. Currently, only "A" is used to hold your collection of
  virtual floppies (and you can't access it as a partition).
- A DSR called LD that will load any program file, whether assembly,
  Extended Basic or cartridge dump.
- A DSR named IDE, that lets you configure your drives or the system
  itself. It contains several pseudo-files:
  - `IDE.MASTER` queries the master drive (see [above](#Drive%20ID)).
  - `IDE.SLAVE` queries the slave drive (see [above](#Drive%20ID)).
  - `IDE.CONFIG` configures the system (see [above](#Configuration)).
  - `IDE.BLOWON`, `BLOWOFF`, `BLOWOUT `and `BLOWAWAY `access the
    Blowfish encryption functions (see [above](#Encryption)).
  - `IDE.FLOPPIES` accesses the floppy disk collection (see
    [above](#Collection)).
  - `IDE.TRASH1,` `TRASH2 `and `TRASH3 `accesses the deleted floppies
    (see [above](#Collection)).
- A clock DSR, named CLOCK (by default), that lets you access the
  real-time clock. You can read time and date, set alarms, etc. See
  [above](#clock) for a detailed description.

[DSKx virtual drives
](#DSKx)[LD loader
](#loaders)[Autostart feature
](#autostart)[Opcode \>0A](#%3E0A): load assembly program
[Opcode \>0B](#%3E0B): load Basic program
[Opcode \>0C](#%3E0C): load cartridge

#### DSKx

These DSRs emulate a floppy drive. Their names can be set with
IDE.CONFIG, but should be kept in the range `DSK1 - DSK9 `and
`DSKA - DSKZ`. As always with DSRs, parameters are passed via a
[PAB](../../headers.htm#PAB) (peripheral access block) in VDP memory.

It is also possible to use the `DSK.DISKNAME.FILENAME` syntax, but this
may not work if the TI disk controller is installed: this stupid
controller returns an error if it does not find the disk in drives 1-3.
Thus `DSK.DISKNAME` only works properly when the IDE card is called
before the TI disk controller, i.e. when its CRU is \>1000.

As a remedy to this unfortunate situation, an extra DSR called **DSK@**
is provided. It is completely equivalent to `DSK `and should be used in
the same way: `DSK@.DISKNAME.FILENAME`. Since the TI controller does not
contain `DSK@ `it won't interfer with the search.

An extra DSR called **DSK\*** lets you access files on the same drive
that was last used (by either a DSR or a call). This is usefull when a
program must load data files: by using the `DSK*.FILENAME` syntax, you
ensure that the data files will always be loaded from the drive the
program was on.

The standard opcodes defined by Texas Instruments are all implemented:

\>00 Open file

\>01 Close opened file

\>02 Read record from opened file

\>03 Write record to opened file

\>04 Rewind opened file to a given record

\>05 Load program file

\>06 Save program file

\>07 Delete file

\>08 Scratch record from file (NB this one did not work with the TI
controller. It does now)

\>09 Return file status.

Several improvements introduced with the Horizon Ramdisk have been
included:

It is not necessary to specify the file type (DV80, IF254, etc) with
`OPEN `if the file already exists. If you do not specify a record
length, the file type will be taken from disk, not from what you
specified. Be aware that Basic wants at least one comma after the file
name in the `OPEN `instruction.

Fixed files can now be opened in append mode: the number of the next
record past the end of file will be placed in PAB+6. This will only work
from assembly though, since Basic and Extended Basic are in the stupid
habit of reseting the record number to 0 after `OPEN`.

Adding \>40 to the opcode means that the data buffer specified at PAB+2
is located in CPU memory, not in VDP RAM.

Opcode 8 (scratch record) works both for fixed and variable records.
There is no way to call it from Basic unfortunately.

All opcodes return the file status in PAB+8, (only opcode \>09 did it
with the TI controller). If a device error occured (IO error code 6)
this byte should contain the error code read from the IDE error
register. See my [IDE page](ide.htm#error%20codes) for meanings.

In addition, the following extra opcodes were defined by the Horizon
Ramdisk, and are also supported by IDEAL:

\>0A Load **A**ssembly program

\>0B Load Extended **B**asic program

\>0C Load **C**artridge in a GRAM card

These serve to load and run a program. They are called automatically by
the LD DSR, but you can also use them from your assembly programs.

### The program loaders

#### The LD DSR

The LD DSR can be used to automatically load and run an assembly
program, in the form of one or more memory dump files in
Editor/Assembler option 5 format. It can also trigger execution of an
Extended Basic file, or load a cartridge into a GRAM device, using the
GramKracker file format. Remember that this is a DSR (a device) not a
subprogram, so the syntax is:

    DELETE "LD.n.filename"

**`DELETE `**is used here as an exemple. Actually, any file operation
will do (e.g. `OPEN`, `OLD`, `SAVE`, `RUN`, and `DELETE`).
**n** is the drive number. If you did not name your drives following the
DSKn convention, n is the number you have assigned to the drive plus 48
(the ascii code for '0').
**filename** can be a simple file name, or a directory path ending with
a filename: `LD.3.DIR1.DIR2.DIR3.MYFILE`

Alternatively, you may use the disk name:

    RUN "LD.@.diskname.filename"

The **@** sign in place of a drive number instructs LD to look for a
vitual floppy in your collection (by analogy with `DSK@`).
**diskname** is the name of the desired virtual floppy.
**filename** is a simple file name or a full directory path.

####  Autostart feature

You have the option to automatically cause LD to load a given program at
power-up time. The program name can be selected with IDE.CONFIG, or with
one the the subprograms HAO, HAF and HAB (respectively standing for
**H**ard-drive **A**utostart **O**n, o**F**f and **B**oot).

    CALL HAO

Toggles autostart on: each time you reboot your TI-99/4A the selected
program will be called in place of the "Texas Instruments Home Computer"
colorfull screen.

    CALL HAO.diskname.filename

Same as above, but lets you change the file that should be called when
booting. Note that you must specify a diskname (not a DSKn drive) and a
filename with no subdirectories inbetween.

    CALL HAF

Toggles autostart off. Note that `CALL HAF.diskname.filename` can also
be used.

    CALL HAB

Toggles autostart on for only one time: when you do a hard reboot (i.e.
turn the PE box power off ). This is usefull if you want to load a
cartridge in your GRAM card at power-up time: obviously, there is no
need to reload it each time you press the \ key. On the other
hand, if power is turned off, the cartridge will likely be wiped out of
the GRAM card, and you'll need to reload it. Since IDEAL will also be
reloaded in such a situation, it will automatically load your cartridge.

Here also, `CALL HAB.diskname.filename` is legal.

Note: the autostart file can also be set with the configuration manager
(see [above](#Auto%20&%20XB%20files)).

####  Opcode \>0A. Load assembly program

From assembly language, you have the option to use three special opcodes
to load a file with any "DSKn" DSR.

Opcode \>0A loads a program file in EA5 format, i.e. that starts with a
6-byte header in the form:

\>0000: flag
\>0001: bank
\>0002: size
\>0004: loading address

If **flag** is not \>00xx, loading continues with a file having the same
name but for the last character that is incremented (e.g.
`MYFILE1 `becomes `MYFILE2`). Execution normally starts at the beginning
of the first file.
**Bank** is used to load a program into the IDE card SRAM banks. It
should contain the bank number, plus 32 (because banks 1 through 31 are
reserved for GRAM cards). Note that, when execution begins, the last
bank is on not the first one!
**Size** specifies the number of bytes to load. It can be ignored,
depending on the PAB (in which case the whole file is loaded).
**Address** is the address where to load the program. It can be
superceeded by the PAB.

Some parameters in the PAB have unusual meanings:

Start address
(not used)
Loadable size
DSKn.FILENAME

**PAB+0**: Contains the opcode (\>0A)
**PAB+1**: Should be 0. Will contain error flags if an error occurs.
**PAB+2**: Address where to start execution. \>0000: at the beginning of
the first file. \>FFFF: load but don't execute.
**PAB+6**: If this word contains 0 (or \>8000) no size checking will
occur. Otherwise, the file will only be loaded if its size (as specified
in the file FDR) is smaller or equal to this value. The \>8xxx bit has a
special meaning: when set, the number of bytes is taken from `SIZE`, the
second word in the file. When the bit is reset, this word is ignored and
the whole file is loaded.

####  Opcode \>0B. Load Extended Basic program

This opcode only works if the Extended Basic cartrige is plugged in, or
is emulated by a GRAM-card. It launches Extended Basic and snoops on it
via a hook to the interrupt routine. When it sees that Extended Basic is
about to load "`DSK1.LOAD`" it patches the filename in memory and
replaces it by the name of the program to be run. Nifty isn't it? (I
whish I could claim this trick as mine, but it's lifted from the Horizon
Ramdisk ROS and they lifted it for someone else).

Note that execution cannot return to the caller once Extended Basic is
done.

If IDEAL does not find Extended Basic, but you have a GRAM card and you
have the Extended Basic cartridge stored somewhere on the hard disk, the
program will load the cartridge into the GRAM card and the proceed with
launching the XB program. For this to happen however, you must use
IDE.CONFIG to declare the location of the XB cartridge (see
[above](#Auto%20&%20XB%20files)).

Only a few bytes in the PAB are actually used with opcode \>0B:

(not used)
(not used)
(not used)
DSKn.FILENAME

PAB+0: Contains the opcode (\>0B)
PAB+1: Should be 0. Will contain error flags if an error occurs.

####  Opcode \>0C. Load cartridge.

This opcode loads one or more program files into a GRAM device. The
files should follow the Gram-Kracker format, i.e start with a 6-byte
header in the form:

\>0000: flag
\>0001: bank
\>0002: size
\>0004: address
\>0008: option

If **flag** is not \>00, loading continues with another file. The second
file has the same name than the first one, with an extra "1". For the
following files, the "1" is incremented. So the sequence would be:
`MYFILE`, `MYFILE1`, `MYFILE2`, etc.
**Bank** specifies the GRAM area into which the file should be loaded.
Note that this information is redundant, since the address is already
part of the header. Alternatively, bank can specify a page in the
cartrige RAM space (\>6000-7FFF). Standard cartridges and GRAM cards
have only two RAM pages but the german GramKarte allows for 16. In
general the lower part (\>6000-6FFF) is identical in each bank, but
there may be exceptions...
**Size** specifies the number of bytes to load.
**Address** it the address (in GRAM or RAM) where to load the data.
**option** corresponds to byte 4 in the GRAM, which is normally zero. If
it contains \>A5 the GRAM device will be turned off after loading. If it
contains \>5A only the RAM banks in the GRAM card will be turned off
after loading.

Here is a summary of the valid values for "bank":
\>01: GRAM at \>6000
\>02: GRAM at \>8000
\>03: GRAM at \>A000
\>04: GRAM at \>C000
\>05: GRAM at \>E000
\>06: GRAM at \>0000 (caution: conflicts with console GROMs)
\>07: GRAM at \>2000 (ditto)
\>08: GRAM at \>4000 (ditto)
\>09: RAM bank 1 (\>6000-7FFF)
\>0A: RAM bank 2 (normally only \>7000-7FFF, but some device may allow
\>6000-7FFF).
\>0B: RAM bank 3 (only on some devices)
etc.
\>18: RAM bank 16

Some parameters in the PAB have a special meaning:

GRAM base
(not used)
(not used)
DSKn.FILENAME

PAB+0: Contains the opcode (\>0C)
PAB+1: Should be 0. Will contain error flags if an error occurs.
PAB+2: If this word contains \>98xx, this value is used as a GRAM base.
Otherwise, the current GRAM base stored at \>83FA is used. If \>83FA
does not contain a valid \>98xx value, the default base \>9800 is used.

#### Gram card-specific routines

IDEAL 1.0 currently supports only the german 128K GramKarte and the
P-Gram+ card. IDEAL's gram-card management routines are all calling a
card-specific subroutine, that resides in a separate file. For the
German 128K Gram-Karte, rename `IDEAL/GK` as `IDEAL/G` (this is the
default). For the P-Gram+ card, rename `IDEAL/PG` as `IDEAL/G`.

If you have another GRAM device than the above, you can write yourself a
new version of this subroutine and IDEAL will use it to control your
device. This routine should be assembled as an AORG segment, to reside
at addresses \>5900-5B00. As it is called by a BLWP instruction the
first two words (at \>5900 and \>5902) should be the workspace and the
routine address. You can place your workspace in the same memory area,
between \>5904 and \>5B00. I would advise you to use the RAG linker to
generate the final "program" file, since the TI loader cannot load a
DF80 file at \>5900. Be aware that your routine will not be able to call
DSRs or other routines in the gram-card ROM since these would conflict
with the IDE card DSR space.

The default gram-card CRU, as set with `IDE.CONFIG,` can be retrieved
from the caller's R1 with a `MOV @2(13),R12` but this is entirely
optional: you could "hard-wire" the CRU inside your code if you want to
(for instance if you have several gram-cards).

IDEAL will call your routine for various purposes. The reason for the
call is found in the caller's R0, that you can retrieve with a
`MOV *R13,R0`.

- If R0 is a positive number between \>00 and \>08, IDEAL is about to
  load data into GRAM memory. R0 is the number of the target GROM (see
  "bank" above for valid values). Your routine should write-enable the
  corresponding memory, The GROM port to be used (e.g. \>9800) can be
  found in the caller's R8, i.e. at @16(R13). You can change it if
  necessary, but do not modify any other register in the caller's
  workspace.
- If R0 is \>09 or greater, IDEAL is about to load data into the
  cartridge RAM space, at \>6000-7FFF. R0 contains the bank number (\>09
  = first bank, \>0A = second, etc). Your routine should switch on the
  corresponding bank, and write-enable it.
- If R0 is negative, IDEAL is done with loading and lets you decide what
  you want to do with your card. As a clue, the least significant byte
  of R0 contains the fourth byte of the last GROM loaded that contained
  a standard header (i.e started with \>AA). The fourth byte is normally
  unused (\>00) but when creating a GRAM file you have the option to set
  it to a special value to specify loading options:
  - If R0 is \>xxA5, the card should be turned off after loading.
  - If R0 is \>xx5A, only the ROM banks should be turned off.

Once done, you should return to the caller with a `RTWP` instruction,
preceded with an `INCT R14`. Returning without modifying R14 signifies
IDEAL that the current file should not be loaded (of course, if they are
more files to come, IDEAL will call again). For instance, since the
console GROMs map at \>0000-57FF, you may want to reject any request to
load at these addresses. If, and only if, IDEAL was calling with \>FFxx
in R0, returning without `INCT R14` will cause a "file error".

Here is a sample Gram-card routine, that handles the German 128K Gram
Karte:

*  User-defined GRAM card routines for GramKarte
*  Common entry point, loading code in *R13
*----------------------------------------------------
* CRU bit   Meaning
* -------   --------
* Bit 0     1=DSR on
* Bit 1     1=banks on
* Bit 2     GRAM-enable 2
* Bit 3     0=GRAM on
* Bit 4     0=write-protect banks
* Bit 5     1=overwrite console GROMs
* Bit 6     1=use default bank (DIP-switch selected)
* Bit 7     0=switch banks 1=access banks
*====================================================
       AORG &gt;5900
GCARD  DATA WREGS,GKARTE      vectors (MUST be right here)
*
GKARTE INCT R14               always ok: skip JMP upon return
       MOV  @2(R13),R12       get default CRU
       MOV  *R13,R11          get loading code
       JLT  GKDONE            &gt;FFxx = done
       CI   R11,&gt;0008         check if RAM-bank or GRAM
       JH   GKBNK             bank
* Load in GRAM
       SBZ  3                 turn GRAM on
       CI   R11,&gt;0006         is it in console space ?
       JL   GK1               no
       SBO  5                 yes: override console GROMs
*      DECT R14               alternative: skip this file
GK1    RTWP
* Load in banks
GKBNK  SBZ  1                 bank read ok
       SBZ  4                 bank write-protected
       SBZ  6                 no default bank
       SBZ  7                 bank switching enabled
       AI   R11,-9            adjust bank #
       SLA  R11,1
       MOV  R11,@&gt;6000(R11)   switch
       SBO  7                 bank switching disabled
       SBO  4                 bank write-enabled
       RTWP
* Done with loading
GKDONE SB   R11,R11           keep only clue code
       CI   R11,&gt;00A5         code for &quot;card off&quot;
       JNE  GK2
       SBO  1                 banks off
       SBO  3                 GRAM off
       SBZ  4                 write-protect banks
       SBZ  5                 console GROMs in charge
       RTWP
GK2    CI   R11,&gt;005A         code for &quot;banks off&quot;
       JNE  GK3
       SBO  1                 banks off
       SBZ  4                 and write-protected
       RTWP
GK3    SBZ  4                 &quot;normal&quot;: write-protect banks
       SBZ  7                 enable switching
       RTWP
*
WREGS  BSS  32                our workspace
       END

Assemble such a file with the TI assembler.

Link the resulting DF80 file with the RAG linker and name the final
program file IDEAL/G.

Edit IDEAL/G with a file editor like DISKU and change the first two
bytes from \>0000 to \>FF22.

Put it where all the other IDEAL/x files are.

##  Subprograms

**Basic calls  **
[WO and WF
](#WO&WF)[DN
](#DN)[HAO, HAF, and HAB
](#HAO+HAF+HAB)[CD
](#CD)**Assembly callable subprograms  **
[\>10 Sector access
](#%3E10)[\>11 Disk format
](#%3E11)[\>12 File protection
](#%3E12)[\>13 File rename
](#%3E13)[\>14 Bulk read
](#%3E14)[\>15 Bulk write
](#%3E15)[\>B0 Rambo manager
](#%3EB0)**Others  **
[DSRs used as calls](#Other%20subs)

### Basic subprograms

There are several subprograms that are designed to be called from Basic
or Extended Basic. You can also call them from assembly, just be aware
that the error code is returned in byte \>8342 and that they skip a word
on return when no error occurs (`INCT R11`).

#### Write-protection

Real floppy disks can be protected in two ways: by software, by changing
the "DSK " string in sector 0 to "DSKP", or by hardware, by blocking the
write-protection notch with some opaque tape. The first mechanism is
ignored by the majority of disk managers, and virtual floppies obviously
don't have any notch! Instead, IDEAL is using a special bit in sector 0
to indicate write-protected floppies. This bit can be set and cleared
with the `IDE.FLOPPIES` utility, or by accessing the disk's extended
catalog.

When a floppy is "inserted" into a virtual drive, its protection status
is read by the drive and comes into effect. However, you have the option
to temporarily override this status and set a different protection, just
for this drive:

**`CALL WO.n`** sets write protection on for drive n

**`CALL WF.n`** turns write-protection off for drive n

Note that this does not affect the write-protection status of the floppy
in your collection, nor does it affect other drives that may be
accessing the same floppy. (Yes, you can load the same floppy in several
drives. But this is quite risky if more than one of them is allowed to
write to it.)

Once you insert a new floppy in the drive, the protection status of that
floppy will come into effect. In other words, whatever command arrives
last is the one that prevails.

####  Drive name

Provided that a virtual drive obeys the DSKn naming convention, you can
change its number n with the following subprogram:

    CALL DN.o.n

where o is the old drive number and n the new number (e.g. `CALL DN.8.5`
renames `DSK8 `as `DSK5`).

An error will occur if a drive with that number already exists in the
IDE card, but IDEAL cannot check for the presence of such a drive in the
floppy disk controller card, nor in a Ramdisk card. In case of
conflicting drive names, the card with the lowest CRU address is the one
who will be accessed.

####  Autostart

As described above, you can toggle the autostart feature on and off the
the following three subprograms:

**`CALL HAO`** turns autostart on

**`CALL HAF`** turns autostart off

**`CALL HAB`** only autostarts at boot time (i.e. once, after power went
off).

Either of these can also be used to change the program to be started at
power-up time:

    CALL HAO.diskname.filename

The selected program may be an assembly program, an Extended Basic
program (in which case the Extended Basic cartridge may be automatically
loaded in your GRAM card, if needed), or a cartridge. Note that
cartridges are loaded, but not executed, unless they contain a power-up
routine, of a foreign language translator.

#### Change disk

As described previously, the CD subprogram can be used in several
manners to change the virtual floppy accessed through a drive:

    CALL CD.n..diskname

This will "insert" the virtual floppy named "diskname" into drive
`DSKn`. Note the double-dot syntax that signals a disk name.

    CALL CD.n.directory

This will insert the disk pointed at by a directory file located on the
disk currently inserted in `DSKn`. This time, there is only one dot
after the drive number, which indicates a directory path.

`CALL CD.n.<`

This will insert the parent of the current vitual floppy in DSKn. A
virtual floppy has a parent if at least one directory file points to it.
If there is more than one, the first one you created is the parent
unless you specified it otherwise. This allows you to organise your
floppies in a tree-wise manner, as on a PC:

    DISK1
    +-HERSTUFF------->DISK5
    |                 +-LETTER1
    |                 +-LETTER2
    |
    +-MYDIR---------->MYDIR
    +-OTHERFILE       +-GAMES----->GAMES
                      |            +-CHESS
                      |            +-HANGMAN
                      |
                      +-NUDIES---->DISK4
                                   +-CLAUDIA
                                   +-ELLE
                                   +-SOPHIE

For example, assume the virtual floppy named `GAMES` is currently
inserted in drive `DSKZ` and you want to access the file called
`LETTER1.` You could do something like:

    OPEN #1: "DSKZ.<.<.HERSTUFF.LETTER1", DISPLAY, VARIABLE 80

Of course, you could also do it this way:

    OPEN #1: "DSK@.DISK5.LETTER1", DISPLAY, VARIABLE 80

or this way:

    OPEN #1: "DSK@.DISK1.HERSTUFF.LETTER1", DISPLAY, VARIABLE 80

####  Assembly subprograms

The standard assembly level subprograms \>10 through \>16 are provided.
A common extra feature is that you can add \>80 to the drive number, to
indicate that the data buffer is located in CPU memory instead of VDP
memory.

#### \>10 Sector I/O

This subprogram reads or writes a sector from the disk to/from a data
buffer that you provide. This buffer is normally in VDP memory, unless
you add \>80 to the drive number. The number of bytes transfered is
always 256.

Input parameters

-
Drive #
0: write
&lt;&gt;0: read
Data buffer address
Sector number

Output parameters

Sector number
-
-
-
Error code

Technical notes

IDEAL stores some information at the end of sector 0 (bytes \>EE-FF)
that is used to manage the floppy disk collection. Since these data are
critical for proper operation, subprogram \>10 does not let you modify
them.

If you have set the time-stamping option while condiguring IDEAL, bytes
\>14 through \>1B may be automatically modified upon writing sector 0.
The current time and date are written to bytes \>18-1B (last
modification stamp) unless bytes \>18-19 contain \>FFFF. If bytes
\>14-15 contain \>0000, the same values are also written to bytes
\>14-17 (creation stamp).

Encryption issues

Subprogram \>10 can tell whether sector 0 is encrypted or not, but it
cannot do it with other sectors. So it just assumes that, if encryption
is on (`DELETE "IDE.BLOWON"`), the whole disk must be encrypted: it
decodes any sector you read and encodes anything you write. If that's
not what you wanted, turn encryption off (e.g. `DELETE "IDE.BLOWOFF"`).

Additionally, if encryption is set with `DELETE "IDE.BLOWOUT"`,
subprogram \>10 will use encryption or not (both to read and write),
according to what was determined the last time sector 0 was read. This
lets you turn encryption on and off from within a disk manager: first
read sector 0, then the sector you want to access. As it turns out,
DISKU's sector editor always reads sector 0 (to determine what the
maximum sector number is) before it lets you edit another sector, so if
you use DISKU encryption is always turned on/off appropriately.

To sector-edit encrypted files on a non-encrypted disk, you will need to
force encryption with `DELETE "IDE.BLOWON"`. This will of course cause
all non-crypted sectors of the disk (except sectors 0+1) to appear
scrambled.

If you try to access an encrypted disk (as determined by a flag in
sector 0) but no password has been entered yet, you will get a disk
error. If you have entered the wrong password, no error is issued but
you'll read only garbage. Note that, as long as you write back exactly
what was read, no data will be lost. Problems only arise when you modify
a sector and save it with the wrong password.

1.

#### \>11 Format disk

This subprogram will NOT format the hard-disk: IDE drives are formatted
in factory and should never be reformatted by the user (it may mess up
the address translation mechanism). What subprogram \>11 does is simply
to create a new virtual floppy and add it to your collection..

Input parameters

-
&gt;A5 flag
Diskname ptr
Density (1/2)
Density (1 or 2) and number of sides (1 or 2) are combined so as to
choose among 3 possible sizes:
SS/SD: 360 sectors (90K)
SS/DD and DS/SD: 720 sectors (180K)
DS/DD: 1440 sectors (360K).

Byte \>834D should contain \>A5 if you wish to pass a diskname in the
buffer pointed at by \>834E. The name must be exaclty 10 characters in
size, so pad the string with trailing spaces if necessary. If this name
already exists in your floppy collection, a "file error" is issued. If
the first character in the name is not printable (ascii 33-127) the
filename is considered invalid and IDEAL will coin a dummy filename in
the form "!xxxx" where xxxx is an arbitrarily chosen hexadecimal number.
This also happens if the flag byte \>A5 is not found in \>834D. This is
generally the case when you use the "format" command of a disk manager:
it creates a noname disk and then names it by modifying sector 0.

Output parameters

# of sectors per disk
-
-
-
Error code

If encryption is on (i.e. `DELETE "IDE.BLOWON"`) the new disk will be
automatically encrypted with the current password. This will not occur
if encryption is set as "output-only" (i.e. `DELETE "IDE.BLOWOUT"`).

#### \>12 File protection

This routine is used to modify the protection status of a file, i.e
whether the file can or cannot be deleted and modified.

Input parameters

Drive #
0: unprotected
&gt;FF: protected
Filename ptr
-

The filename must be a left-justified, 10-char string. Add as many
trailing spaces as necessary.
Output parameters

-
-
-
Error codes

#### \>13 Rename file

This routine changes the name of a file on a given drive.

Input parameters

Drive #
-
New name ptr
Old name ptr

Filenames must be 10 character strings, with as many trailing spaces as
necessary. Beware that no check is made to ensure that the new name is a
valid filename!
Output parameters

-
-
-
Error codes

#### \>14 File input

This routine reads data from a file sector-wise, irrespective of its
type, record length, etc. For instance, it can read a "program" file by
chunks of 256 or 512 bytes (or whatever size is convenient).

Input parameters

Drive #
# of sectors
Filename ptr
Info ptr (&gt;83xy)
-

\# of sectors: 0 = Get file info (fills the structure below, using data
from the file's FDR)
\<\>0 = Read this number of sectors (starting from the first sector
indicated in the structure below).

VDP buffer address
First sector #
File type
Recs / sector
EOF offset
Rec lenght
# of rec
-
Output parameters

-
# of sectors read
-
Error codes

Result of get file info (# of sectors=0):

-
-
File type
Recs / sector
EOF offset
Rec lenght
# of rec
-

####  \>15 File output

This routine writes data to a file sector-wise, irrespective of its
type, record length etc.

Input parameters

Drive #
# of sectors
Filename ptr
Info ptr (&gt;83xy)
-

\# of sectors: 0 = Create file (using info in the structure below to
create the FDR)
\<\>0 = Write this number of sectors (from the first sector indicated in
the structure below).

VDP buffer address
First sector #
File type
Recs / sector
EOF offset
Rec lenght
# of rec
-
Output parameters

-
# of sectors writen
-
Error codes

####  \>16 and FILES: Number of files

With the TI floppy controlle, these two subprograms serve to change the
number of files that can be opened at a time. CALL FILES(n) is used from
Basic, and subprogram \>16 from assembly. This was necessary because the
DSRs on the floppy controller card used VDP memory to buffer opened
files, and space is quite limited over there (that's where Basic
programs and variables resides, among other things).

The IDE card has its own memory chip and uses it both to store the DSR
and to buffer opened files. Therefore it is not necessary to constantly
jungle with VDP memory usage, and subprogram \>16 and FILES are not
implemented in IDEAL.

There is a way to specify how many opened files you allow, when
configuring your system with IDE.CONFIG. By default, this number is 15,
so at any given time you can have upto 15 files opened on the hard-drive
.The absolute maximum value depends on the size of your SRAM.

####  \>B0 Rambo access

This subprogram was introduced in the Horizon Ramdisk operating system.
It allows access to Rambo banks, i.e. memory in the area \>6000-7FFF.
The subprogram actually consists in 3 subfunctions, selected by the
value in \>834C.

##### Page count


-1: error

Returns the total number of pages available in all Rambo devices.

Important note: if you have an Horizon Ramdisk and an IDE card, you
should run this subprogram first. It will let IDEAL bias the page number
by whatever pages there are on the Ramdisk (e.g. if there are 100 pages
in the Ramdisk, page 101 is the first one in the IDE card).

#####  Page selection


-1: error

Selects a given Rambo page. Returns the CRU of the Rambo device in
\>834A and the CRU value used in \>834C. Note that Rambo pages are not
selected by the CRU in the IDE card, so the value returned in \>834C can
only be used to turn Rambo mode on, NOT to switch pages: always use
subprogram \>B0 for this.

##### Rambo off

|        |       |        |
|--------|-------|--------|
| Word   | Input | Output |
| \>834A | \-    | \-     |
| \>834C | -1    | -1     |
| \>8350 | \-    | 0      |

Turns off Rambo mode.

#### DSRs used as subprograms

- The DSRs `IDE.BLOWON`, `IDE.BLOWOFF`, `IDE.BLOWOUT` and `IDE.BLOWAWAY`
  can also be accessed as subprograms, for instance with a Basic
  `CALL`..
- So does the `LD.filename` loader device.
- Conversely, the `CD `subprogram can be called as a DSR.

##  Modifications to the TI disk format

As far as possible, I have tried to stick to the floppy disk format
established by Texas Instruments. However, I had to make a few
modifications to allow for new features. They are summarized here.

#### Sector 0

This sector holds the disk directory just like with the TI format, but
most of the reserved bytes have now been put to use. Bytes \>EE through
\>FF are protected by IDEAL so they cannot be modified with a sector
editor.

    Bytes   Contents
    >14-15: Time of creation.
    >16-17: Date of creation.
    >18-19: Time of last modification.
    >1A-1B: Date of last modification.

    >EE   : Flags (>10=archive, >02=encrypted, >04=720 sectors, >08=1440 sectors, >20=protected).
    >EF   : Node balance (>FF=heavy left, >00=balanced, >01=heavy right, >80=deleted).
    >F0-F1: Number of items in the left subtree.
    >F2-F5: LBA of left subtree(>0000 0000 if none).
    >F6-F9: LBA of right subtree.
    >FA-FD: LBA of parent directory.
    >FE-FF: Sector # for FDR of pointer file on parent directory.

N.B. Sector 1 follows the TI format with no modification.

#### File Descriptor Records (FDRs)

There is one such sector for each file, that contains all the relevant
file informations. These are the sectors listed in sector 1. The
following modifications were introduced.

    Bytes   Contents
    >0A   : Flags (>10=acrchive bit, >02=Encrypted, >08=directory pointer).

    >14-15: Time of creation.
    >16-17: Date of creation.
    >18-19: Time of last modification.
    >1A-1B: Date of last modification.

#### Directory pointer files

A directory pointer is nothing more than an empty file, consisting of
only the FDR. It follows the new FDR format described above. There are
some extra modifications to allow chaining of all pointer files that
refer to the same directory.

    Bytes   Contents
    >20-23: LBA of the current disk (i.e. of sector 0).
    >24-27: LBA of sector 0 for previous link.
    >28-29: Sector number for previous link.
    >2A-2D: LBA of sector 0 for next link.
    >2E-2F: Sector number for next link.
    >30-31: Number >0000 (used to detect when it's encrypted).

Revision 1. 7/28/00. Preliminary, not for release.
Revision 2. 5/28/01. Ok to release (about time!).
Revision3. 8/25/01. Minor modifs to "Getting started".
[Back to the TI-99/4A Tech Pages](titechpages.md)
