# The Basic interpreter

Most of the ROM and GROM memory in the TI-99/4A is devoted to the Basic
Interpreter. One could even consider that TI-Basic is operating system
of the TI-99/4A.

This interpreter is written in GPL, itself an interpreted language, with
a bunch of assembly language routines. This has the unfortunate
consequence of making TI-Basic extremely sluggish, as the double
interpretation process takes a lot of time. On the other hand, one could
argue that it makes programs more compact (although I'm not fully
convinced of that).

I don't have the pretention of understanding the TI-Basic interpreter.
In this page, I summarized information that I gathered from several
sources and that (I hope) could be usefull to somebody studying TI-Basic
"under the hood".

**Structure of a Basic program  **
[VDP memory
](#VDP%20usage)[Statement list
](#statements)[Tokens
](#tokens)[Line number table
](#line%20numbers)[Symbol table
](#symbol%20table)[String space
](#String%20space)[PAB chain
](#PABs)[Value stack
](#variables)[Scratch-pad usage](#scratch-pad)

**The Basic GROMs  **
[Tables
](#Basic%20tables)[\_PARSE
](#PARS%20table)[\_EXEC
](#EXEC%20table)[\_CONT
](#CONT%20table)[\_NUD
](#NUD%20table)[\_Tokens](#Token%20tables)

[Routines](#Basic%20routines)

##  Structure of a Basic program

A TI-Basic program consists in a line number table and a list of Basic
statements. It is generally stored at the top of the VDP memory
(although Basic programs can be stored in GRAM/GROM). The statement list
comes just under the area reserved by the disk controller for file
buffers, and grows downwards. The line number table comes under it.

Under the program come all the variable and constants it uses: the value
stack, the symbol table, the string space and the PAB space.

### VDP memory usage with TI-Basic



&gt;02FF
+ Char pattern table (chars &gt;00-60)
&gt;031F
&gt;036F
(human-readable Basic &lt;=&gt; tokens)
&gt;03C0
&gt;03DF
&gt;07FF
VDP roll out buffer

@&gt;836E
@&gt;8318
@&gt;8332
@&gt;8370

###  Disk file buffers

The TI disk controller card reserves space at the top of the VDP RAM at
power-up time. Other controllers may or may not do so. The TI routines
are written in such a way that several controller cards can reserve
space under each other.

The last free address, just under the bottom of these buffers, is stored
in \>8370. Normally it will be \>37D7, with CALL FILES(3). If you change
the number of files, add or substract 518 bytes (\>206) per file.

###  Basic statement list

This list starts at the top of the VDP memory, just under the area
reserved by the disk controller. The list grows downwards, with the last
line that was typed in at the bottom. Which means that the lines are not
sorted by number (the line number table will take care of that). During
Basic execution, [XML \>1B](gpl2.htm#xml1b) can be used to get the next
token and place it in \>8342.

Each statement begins with a lenght byte and ends with a \>00 byte. All
TI-Basic keywords are replaced with 1-byte codes known as
"[tokens](#tokens)". This both saves space and speeds up execution. The
"crunch buffer" at \>0320-036F is used to place the decoded Basic
statment and perform the conversion operations.

#### Scratch-pad addresses

\>837C contains a pointer to the next token to be processed in the
current statement.
Byte \>8342 contains the value of the previous token fetched from the
statement.
\>8332 points to the top of the [line number table](#line%20numbers),
just below the statement list.
\>8370 points to the last byte in the statement list.

#### Example

Here is a exemple of how a TI-Basic statement is encoded.

100 CALL MYSUB(A,"TEST2",U\$,512).

    Address Token/chars Meaning
    >37BA    >1D        Line size
    >37BB    >9D        CALL
    >37BC    >C8        Unquoted string
    >37BD    >05        String length
    >37BE    MYSUB      The name is, of course, not encoded
    >37C3    >B7        (
    >37C4    A          Variable names are not encoded either
    >37C5    >B3        ,
    >37C6    >C7        Quoted string "..."
    >37C7    >05        String length
    >37C8    TEST2      Content of the string
    >37CD    >B3        ,
    >37CE    U$         Another variable
    >37D0    >B3        ,
    >37D1    >C8        Unquoted string
    >37D2    >03        String length
    >37D3    512        Numeric constants are passed as strings
    >37D6    >B6        )
    >37D7    >00        End-of-line mark

####  TI-Basic tokens

A TI-basic program is not stored as such in memory. Instead, each line
is "crunched" into 1-byte tokens, as much as possible. All token have a
value of \>80 or above, which allow to quickly distinguish them from a
variable name or a number (since these are made of ascii characters).
The exceptions are those token that correspond to keywords that cannot
be part of a program (e.g. NEW, RUN, etc).


string
string
number

###   Line number table

The line number table begins right below the [statement
list](#statements) and grows downwards in VDP memory. The line numbers
are arranged in numerical order from top to bottom. Each entry consists
in two words: the line number and the address of that line in the
statements list. This is the address of the first executable byte in the
statement, i.e. after the size byte.

#### Scratch-pad addresses

Word \>832E: Current line entry in the line number table
Word \>8330: Bottom of the line number table
Word \>8332: Top of the the line numer table
Word \>8336: Entry for the line containing the next DATA element

#### Example

100 CALL MYSUB(A,"TEST2",U\$,512)
110 STOP

The line number table for the above program would be:

    Address Contents  Meaning
    >37AF : >0063     Line number (110)
    >37B1 : >37B8     Pointer to the line in the statement list
    >37B3 : >0064     Line number (100)
    >37B5 : >37BB     Pointer to the line in the statement list

###   Symbol table

This table is used to store all variables used in a Basic program. Each
entry is at least 8 bytes (i.e. 4 words) long, but can be much longer if
needed. For instance, entries for arrays contain a size word for each
dimension, followed by pointers for each elements.

[XML \>13](gpl2.htm#xml13) and [XML \>16](gpl2.htm#xml16) can be used to
search the symbol table for a given symbol. Both place a pointer to the
symbol in the symbol table at \>834A. Then [XML \>14](gpl2.htm#xml14)
can be called to generate an 8-byte description for this symbol.
[XML \>15](gpl2.htm#xml15) can be used to modify the value of the
variable, after pushing the description on stack with [XML
\>17](gpl2.htm#xml17).


Floating point value (4 words)
Size for each dimension. Floating point values.
Size for each dimension. Pointers to strings.

Ptr to name: points to a location in the [statement list](#statements)
where the variable name is spelled out. E.g. X=25.
Ptr to next entry: allow to walk the symbol table by linking from a
variable to the next. The last entry contains \>0000.

#### Scratch-pad addresses

Word \>833E: Bottom of the symbol table
Word \>8330: Bottom of the [line number table](#line%20numbers) (which
is just above the symbol table).

#### Example

100 X=25
110 A\$="STRING IN A\$"

This sample program generates the following symbol table:

    Address  Content
    >3778 :  >80 02 >3780 >37AB >3759
    >3780 :  >00 01 >0000 >37BE >4019 >0000 >0000 >0000

\>37AB and \>37BE are addresses in the statement list where the variable
names A\$ and X are mentionned. The sizes of their names are 02 and 01
respectively.
\>3759 is an address in the string space, pointing at the S in "STRING
IN A\$".
\>4019 0000 0000 0000 is the floating point value for 25.0

Here is another example, that deals with arrays:

100 DIM A\$(2,3)
110 A\$(0,0)="TEST"
120 A\$(0,1)="THIS"
130 A\$(1,1)="THAT"
140 A\$(1,2)="SOMETHING ELSE"

    >8202 >0000 >37CC >0002 >0003   >3735 >372D >0000 >0000  >0000 >3725 >3713 0  0 0 0 0
     || |    |     |     |     |    A$(0,0) (0,1) (0,2) (0,3)  (1,0) (1,1) (1,2)  ...  (2,3)
     || |    |     |     |     '- Second dimension  '-----'------'-- empty strings
     || |    |     |     '------- First dimension
     || |    |     '- Ptr to name in program
     || |    '------- Next entry (none here)
     || '- Name size (A$ is 2-char long)
     |'--- Number of dimensions (2 here)
     '---- String flag

For a string array, the values following the dimensions (\>3735, \>372D,
etc) are pointers to the strings in the string space. If the string is
empty, there is nothing is the string space and the pointer is \>0000.

For a numerical array the structure is similar, except that the string
pointers are replaced with numerical values.Each value is 8-byte long in
floating point format.

###  String space

Strings are saved in the string space at the bottom of the VDP memory.
The strings accumultes downwards. When you change the value of a string
variable, a new string is created in the string space. The old one
remains in place, but it not used anymore.

The GROM routine \>0038 can be used to allocate space for a new string
in the string space. If necessary, it will call the "garbage collection
routine" to free more space by getting rid of unused strings.

Each string is preceded and followed by a size byte. In addition, each
string is preceded with a pointer to its entry in the [symbol
table](#symbol%20table) (to make it easier to change its pointer when
the string has to be moved). This pointer is \>0000 for unused strings
that can be deleted by the garbage collection routine.

#### Scratch-pad addresses

Word \>8318: Top of the string space.
Word \>831A: Bottom of the string space.
Word \>831C: Temporary pointer to a string.

#### Example:

100 PRINT "THIS IS A TEST"
110 A\$="STRING IN A\$"
120 A\$="NEW STRING"

This sample program generates the following string space:

    Address Contents                      Comments
    >374D : >3783 >0A NEW STRING >0A      New version. >3783 points to >3750 in A$.
    >375B : >0000 >0C STRING IN A$ >0C    Old version: can be deleted
    >376B : >0000 >0E THIS IS A TEST >0E  Temporary string used by PRINT

The symbol table contains only one entry, the one for A\$:

    Address Contents                      Comments
    >377D : >80 >02 >0000 >37B3 >3750     >3750 points at the N in "NEW STRING"

###   PAB chain

In Basic, all Peripheral Access Blocks are linked together, in a list of
the following format:

Bytes 0-1 : link to next PAB (\>0000 in last one)
Byte 2 : file \#
Byte 3 : internal offset (used to write in PAB data buffer)

Byte 4 : opcode
Byte 5 : error/type flags
Bytes 6-7 : data buffer address
Byte 8 : record length
Byte 9 : characters count
Bytes 10-11: record number (relative files)
Byte 12 : screen offset/status returned by opcode \>09
Byte 13 : file name length
Byte 14+ : file name

By that time, you probably realised that bytes 4 to 14 correspond to
bytes 0 to 10 of the PAB used to call a [DSR](headers.htm#PAB): TI-Basic
just adds a 4 bytes header above them.

#### Scratch-pad addresses

Word \>833C: Pointer to the first PAB in the chain

###  Value stack

Basic maintains a stack in VDP memory onto which values can be pushed
with with [XML \>17](gpl2.htm#xml17) and retrieved with [XML
\>18](gpl2.htm#xml18). The internal description of Basic symbols is
fairly complex, since there are many different types of symbols, however
[XML \>14](gpl2.htm#xml14) can be used to generate a description that
always fits in 8 bytes (i.e. 4 words), to be placed on the value stack.
Always use XML \>17 and \>18 to push/pop strings from the stack since
they update the pointers in the description (this is required if a
string was moved while its entry was on the stack, for instance).

Note that it may happen that a program does not require any value to be
placed on the stack.

Here are the formats for the different entries in the value stack, as
created by XML \>14:

Word 2

Floating point number, in radix 100 notation
&gt;6500
&gt;0000
&gt;6500
&gt;6600
&gt;6700
&gt;6800
&gt;6880

**Ptr to entry**: points to the entry for that variable in the symbol
table.
**Dim**: number of dimensions of the array.
**\***: each FOR entry is followed by two numeric entries: the step and
the upper limit values.

###  Scratch-pad usage summary


3=1 Trace, 4=1 Edit mode, 5=1 On warning stop,
6=1 On warning next, 2 + 7 unused.

------------------------------------------------------------------------

Basic GROMs structure

Here is an outline of the content of the Basic GROMs (addresses \>2000
to \>57FF).

### Tables

#### EXEC table

The GPL opcode EXECute can call 34 procedures, their addresses being
stored in a table located at \>1C9C in CPU memory. These procedures may
be in ROM or in GROM, in which case they are written in GPL. For the
latter, the table only contains an index with a \>8000 flag to indicate
a GROM procedure. The index is used to branch inside the [NUD
table](#NUD%20table) located in GROM memory.

NB \>1A2C corresponds to an error code 0

Token
Address
Token
Address

reserved
&gt;1A2C
INPUT
NUD &gt;16
ELSE
&gt;1A2C
DATA
&gt;19E6
: :
&gt;1A2C
RESTORE
NUD &gt;12
IF
&gt;1BB6
RANDOMIZE
NUD &gt;14
GO
&gt;1A8E
NEXT
&gt;1C14
GOTO
&gt;1AFC
READ
NUD &gt;0A
GOSUB
&gt;1AE0
STOP
&gt;1A3C
RETURN
&gt;1B74
DELETE
NUD &gt;3E
DEF
&gt;19E6
REM
&gt;19E6
DIM
&gt;19E6
ON
&gt;1A92
END
&gt;1A36
PRINT
NUD &gt;0C
FOR
NUD &gt;00
CALL
NUD &gt;0E
LET
&gt;1BEA
OPTION
&gt;19E6
BREAK
NUD &gt;02
OPEN
NUD &gt;18
UNBREAK
NUD &gt;04
CLOSE
NUD &gt;1A
TRACE
NUD &gt;06
SUB
&gt;1A2C
UNTRACE
NUD &gt;08
DISPLAY
NUD &gt;3C
UNTRACE
NUD &gt;08



####  PARS table

The GPL opcode PARSe uses a table located at \>1CE2 in CPU memory, that
contains addresses for 38 procedures. These procedures can be in
assembly or in GPL.NUD refers to an entry in the [NUD
table](#NUD%20table) in GROM.
NB. \>1A2C corresponds to an error code 0



#### CONT table

The GPL opcode CONTinue uses an 8 procedures table located at \>1D2E in
CPU memory. These procedures are all in ROM.

|       |         |
|-------|---------|
| Token | Address |
| =     | \>1D5C  |
| \<    | \>1D3E  |
| \>    | \>1D4C  |
| \+    | \>1DEC  |
| \-    | \>1E18  |
| \*    | \>1E24  |
| /     | \>1E30  |
| ^     | \>1E3C  |

####  NUD table

Some of the procedures called by PARSe and EXEC are written in GPL and
stored in the Basic GROMs. They are listed in the NUD table in the form
of `BR G@xxxx` statements (that may well lead to another branch
statement). The NUD table is in GROM memory, at address specified by
word \>8322 (normally \>4E84). The parse and exec tables in ROM only
contain index values whithin the NUD table (plus a flag bit of \>8000 to
indicate a GPL procedure).



####  Token tables

The crunching and expanding routines use a token table located in GROM
at address \>2870. The table is split into several sub-tables, depending
on the lenght of the keywords. For each keyword length, the subtable
ends with a \>FF byte.

The pointers for the different sub-tables are just before them, at
address \>285C:

    Address  Value  Pointer to subtable for
    >285C    >2870  1-byte keywords
    >285E    >288F  2-byte keywords
    >2860    >289C  3-byte keywords
    >2862    >291D  4-byte keywords
    >2864    >2973  5-byte keywords
    >2866    >299E  6-byte keywords
    >2868    >29D0  7-byte keywords
    >286A    >29F1  8-byte keywords
    >286C    >2A16  9-byte keywords
    >286E    >2A2B  10-byte keywords

Tokens C7 to C9 are not part of the token table.

###  Routines

See the NUD table above, for the address of some Basic procedures.

Here are a few other entry points in GROM memory:


NUMBER, OLD, RES,SAVE and EXIT.
0-9..).

Revision 1. 6/13/99 Preliminary, but ok to release
Revision II. 9/12/99 Made some corrections. Added examples.
Revision III. 9/19/99 More examples added (symbol table, value stack).
Revision IV. 3/4/00. Added PARS, EXEC, and CONT tables.

[Back to the TI-99/4A Tech Pages](titechpages.md)
