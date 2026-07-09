# The p-code system

#### Introduction

The idea behind p-code is very elegant: it is an "ideal" assembly
language to be run on a virtual microprocessor. Concretely, it means
that it requires an interpreter, the p-machine emulator or PME, to
emulate this virtual microprocessor. The big advantage is that you can
write p-code "assembly" programs (or compile higher languages into
p-code) and run them on many different machines. The language was
optimized so as to be easy to emulate and to run smoothly on any
machine. Apart from the TI-99/4A there were emulators written for the
Z80, Z8000, 6502, 6509, Motorola 6800 and 68000 (Apple computers),
HP-87, PDP-11, Intel 8080 and 8086 (PCs) , and GA440 microprocessors
(and what on earth is the latter?).

This article is split in two pages. The present one discusses the
p-system operation, another describes the TI-99/4A [p-code
card](pcode.md) and accompanying software.

p-code is a fully relocatable language: all addressing is done relative
to the current instruction or to the beginning of a segment. This allows
for a very flexible operating system, that can load program segments in
memory, move them as needed, reload them from disk at a different
location, etc. The main drawback is that it is slower than real assembly
language since it is interpreted. On the other hand, it is fairly
compact and lends itself easily to compilation of high-level languages
(mainly Pascal).

A typical p-system implementation consists in several layers:

The PME (pseudo-machine emulator) translates p-code into assembly,
handles the stack and registers, etc. All parts of the PME that are not
dedicated to directly emulate p-code form the RSP, or run-time service
package. The section of the RSP that communicates with the BIOS is
called RSP/IO.

The BIOS (Basic Input/Output Subsystem) allows for interfacing the PME
with computer-specific hardware. Generally, this will be the ROM BIOS
(on the TI-99/4A: the cards DSRs) with a thin wrapping layer to
implement a few niceties specific to the p-system.

The operating system loads p-code programs, manages segments and the
memory heap, deals with run-time errors, etc.

Programs compiled in p-code run whithin the operating system. These can
be system-level utilities, such as an Editor or a Compiler, or
user-written programs. It is also possible to run 9900 assembly-language
programs within the p-code system.

[The p-machine emulator (PME)
](#p-machine)[Run-time service package (RSP)
](#RSP)[BIOS
](#BIOS)[Operating system
](#OS)[File formats](#File_format)

##  The p-machine emulator

[Registers
](#registers)[Stack
](#stack)[Procedure nesting
](#proc%20nesting)**Instructions  **
[\_Calls
](#calls)[\_Data access
](#data%20access)[\_Stack manipulation
](#stack%20manip)[\_Arithmetic & logic
](#arithm-logic)[\_Comparisons
](#comparisons)[\_Flow control
](#flow%20control)[\_Arrays and Strings
](#strings)[\_Sets
](#sets)[\_Multitasking
](#multitasking)[\_Miscellaneous
](#misc%20instructions)**Standard procedures  **
[\_Summary
](#Standard%20procedures)[\_Unit-related
](#Unit-related)[\_String-related
](#string-related)[\_Code pool management
](#code%20pool-related)[\_Concurency procedures
](#concurency-related)[\_Miscellaneous procedures
](#misc%20procedures)[\_Compiler usage
](#compiler-related)**Faults and execution errors  **
[\_Faults
](#Faults&errors)[\_Execution errors
](#Execution%20errors)**Data representation  **
[\_Words (byte sex)
](#data%20representation)[\_Long integers
](#Long%20integers)[\_Real numbers](#real%20numbers)

#### Registers

Every microprocessor contains registers, i.e. a small amount of on-chip
memory. These can be general purpose or dedicated registers. For
instance, the PC register on a TMS9900 microprocessor points to the next
instruction to be executed. By contrast, general-purpose registers can
be used by the programmer for any purpose he fancies.

The p-machine emulates only special-purpose registers. To perform any
calculation you must use the memory stack:. push a number on the stack,
push another number, perform an addition, and pop the result from the
stack. This is a bit of a pain, unless you a are a fan of
Hewlett-Packard calculators and their reverse-polish notation. On the
other hand, it sure makes the p-machine easy to implement on any kind of
system.

The dedicated registers are implemented in the host system memory and
hold various pointers and data required for proper operation of the
p-machine. They are normally used only by the p-machine emulator itself,
however there are two p-codes that let you fool around with them: LPR
and SPR.

**IPC  **
This register contains a pointer to the next p-code to be executed.

**SP  **
Points to the word at the top of the stack. This pointer changes after
execution of almost any p-code whenever something is push on, or poped
off the stack.

**CURPROC  **
This register contains the number of the current procedure (0-255) in
the current segment.

**MP  **
Contains a pointer to the current activation record ([MSCW](#MSCW)).
This is required to access local variables that are allocated on the
stack when a procedure is called. Global variables are accessed via
BASE, or indirectly via EREC.

**CURTASK  **
Points to the current task information block ([TIB](#TIB)).

**READYQ  **
Points to the TIB at the top of the list of tasks ready to run.

**EREC  **
Points to the current environment record ([E_Rec](#E_Rec)), which
changes every time a call is made to a procedure in a different segment.
The E_Rec contains many usefull values that may also be copied into a
dedicated register. This makes the implementation of such registers
optional since a given p-machine emulator (a.k.a. PME) can access them
indirecly via EREC.

**EVEC  **
Points to the current environment vectors. This is an exemple of a
redundant register, since its content can be found in the E_Rec.
However, it should be implemented on any PME.

**IORESULT  **
Contains the completion code resulting from the last I/O operation
(0-255). This is the only register that can be accessed direcly by the
system, as it is located in the SYSCOM memory area.

**BASE  **
Contains a pointer to the global variables. This register is optional
since its value can be found via EREC.

**SIB  **
Contains a pointer to the current segment information block. Optional,
since such a pointer can be found in the E_Rec.

**CURSEG  **
Points to the current segment in memory. Optional since such a pointer
is found in the SIB.

####  Stack

As the p-machine virtual processor does not comprise any general-purpose
register, all data manipulation must be performed on the stack. This is
why basically any p-code affects the stack. For instance, to add two
numbers you would push each of them on the stack (e.g. with LCB, load
constant byte), add them (with the ADI p-code), and retrieve the result
from the stack (here, using the SRO p-code to store it in global
variable \# 5):

                                            <-SP
                            <-SP    |  34  |                <-SP
            <-SP    |  10  |        |  10  |        |  44  |                <-SP
    |//////|        |//////|        |//////|        |//////|        |//////|
             LCB 10          LCB 34           ADI            SRO 5

The stack is used by the p-machine to implement local variables when a
procedure is entered. The caller must also place the procedure
parameters on the stack prior to the call, and reserve room for the
return value if the procedure is a function.

####  Procedure nesting

From the structure of the p-code language, you can tell it was invented
by Pascal programmers: provision is made for procedures that are local
to other procedures (i.e. nested into them) and a lot of p-codes are
devoted to access these procedures or their variables. Let me illustrate
this with an exemple:

    segment SEG1

      int VARG1

      proc GLOB1
           proc LOC1
                int VAR1
           end-of-LOC1

           proc LOC2
           end-of-LOC2
      end-of-GLOB1

      proc GLOB2
           proc LOC3
                int VAR3
                proc LOC31
                     int VAR31
                     Proc LOC311
                     end-of-LOC311
                <>
                end-of-LOC31
           end-of-LOC3

           proc LOC4
                int VAR4
           end-of-LOC4
      end-of-GLOB2

    end-of-SEG1

In this example, GLOB1 and GLOB2 are global procedures, VARG1 is a
global variable. LOC1 and LOC2 are procedures local to GLOB1, whereas
LOC3 and LOC4 are local to GLOB2. LOC31 is a procedure local to LOC3 and
LOC311 is local to LOC31. Note that at each level, there can be
variables (in this case, integers) local to the procedure.

Assume the program has reached the point indicated by \<\<you are
here\>\>, in procedure LOC31. From there, you could:

- Call the procedure LOC311, which is local to the current procedure
  (i.e. LOC31): use the CLP p-code (call local procedure).
- Call global procedure GLOB1 (or GLOB2: recursive call) with the p-code
  CGP (call global procedure).
- Call a procedure by walking up the nesting structure with the CIP
  p-code (call intermediary procedure). CIP 0 is the same as CLP. CIP 1
  calls a procedure that is 1 level above the current one, i.e. its
  lexical parent LOC3, CIP 2 calls the procedure two levels above, etc
  (in our case, CIP 2 calls GLOB2, and is thus equivalent to CGP).
- In Pascal, you cannot call LOC1 because it is local to GLOB1, which is
  not a parent of the current procedure. In p-code however, you could do
  it with any of the above calls since the procedure is called by
  number. But the [MSCW](#MSCW) will contain a bad static link and you
  are up for trouble if you try accessing local or intermediary
  variables from inside the called procedure..
- You could also access procedures located in another segment, with
  p-codes CXG, CXL, and CXI. (Again, the last two would be illegal in
  Pascal).

Similarly you can access the following variables:

- VAR31 wich is local to the current procedure, by using "local" p-codes
  such as LLA, LDL, STL, etc.
- VARG1 with global variable p-codes (LAO, LDO, SRO, etc).
- VAR3 with "intermediary" p-codes, that walk up the nesting levels:
  LDA, LOD, SOD. For VAR3 you would use LOD 1,1 to step up one level.
- You can't access VAR4 from where you are, because it is local to LOC4
  which is not a parent of the current procedure.
- You could also access global variables located in another segment, but
  not local variables.

OK, I hope you got the picture, let's procede with p-code instructions.

###  Instructions

A p-code (pseudo-opcode) is always 1 byte long, but it can be followed
by one or more arguments. There are several types of arguments:

- \ Unsigned byte. A number between 0 and +256
- \ Signed byte. A number between -128 and +127. Negative numbers
  are expressed as two's complement (\>FF = -1, \>FE = -2... \>80 =
  -128).
- \ Don't care byte. A number between 0 and +127. The most
  significant bit is always 0.
- \ Word. This is a two-byte parameter that contains a number
  between -32768 and +32767. Negative numbers are in two's complement
  representation (\>FFFF = -1, etc)..
- \ Big. This parameter can be either one or two bytes long. With
  one byte, it represents a number from 0 to +127 (i.e. the leftmost bit
  is 0). With two bytes, it represents a number between 0 and +32768. So
  as to distinguish the two possibilities, the leftmost bit is set to
  indicate a two-byte parameter (e.g. \>8101 is 257).

For your convenience I have arranged the p-codes in several families:

#### Calls

These p-codes are used to call a procedure. Before executing such an
instruction, you should prepare the stack with the adequate parameters,
as illustrated below. It is the responsability of the called procedure
to pop each parameter from the stack and to leave the adequate result on
it (if any). Note the practice of reserving space for the result before
the first parameter. This generally will be one or two words, but may be
upto four words, depending of the format used for real numbers.

                                                <                  <
                                       | MSCW  |          | MSCW  |
                             <         | data  |          | data  |
                    | param |          | param |   ...    | param |                 <
             < ...  |   0   |          |   0   |          | result|        | result|
    |///////|       |///////|          |///////|          |///////|        |///////|
             prepare          CXG 2,31         called proc           RPU            caller can
             for call         calls a proc     does its job         return          use result

Within the program stream, each procedure entry point consists of two
data words that immediately precede the first p-code to be executed.
These are **exit_ic** that points to some (optional) exit code to be
execute upon returning from the procedure and **data_size** that
indicated how many bytes must be reserve on stack to store the procedure
local variables. This space is marked as "data" in the above
illustration.

You may have wondered what MSCW stands for.
That's "Mark Stack Control Word", also known as "activation record". A
mark stack consists in five words:

MSSTAT
MSDYN
MSIPC
MSENV
MSPROC

MSPROC hold the procedure number of the caller, MSIPC is a
segment-relative pointer to the point of call, MSENV contains a pointer
to the E_Rec of the caller (i.e. its segment). MSDYN points at the MSCW
of the caller and MSSTAT points at the MSCW of the lexical parent (i.e.
the procedure into which the current procedure is nested, if any). The
latter will be needed to access variables that are not local to the
procedure.

When a call is performed, the PME allocates space on the stack and
initialises the MSCW. It sets IPC to point at the first p-code in the
procedure, and update the CURPROC, EREC and EVEC registers. If there
aren't 40 words free on the stack after data and MSCW have been
reserved, a stack fault is issued that will be handled by the system.

There are several p-code calls, depending where the procedure is
located:

**CGP \  **
Call Global Procedure. The procedure number in passed with UB, an
unsigned data byte included in the code stream, just after the CGP
p-code itself.

**CLP \  **
Call Local Procedure, i.e. a procedure nested into the calling
procedure. Again, UB is the procedure number and follows CLP in memory.

**CIP \\  **
Call Intermediary Procedure, i.e. a procedure which is a lexical parent
of the current one. DB is a value from 1 to 127 that indicates how many
nesting levels should be walked back to the target procedure, from MSCW
to MSCW. UB is the procedure number.

**SCIPn \  **
Where n is 1 or 2. This is a short form of CIP, that calls procedures
\, and sets the MSSTAT field of MSCW to the lexical parent (for
SCIP1) or grandparent (for SCIP2) of the calling procedure. It is
shorter than CIP since the nesting level is part of the opcode, so it
uses two bytes instead of three.

**CXG \,\  **
Call eXternal Global procedure, same as CGP but targets another segment.
UB1 is the segment number. If it's not in memory yet, a segment fault is
issued so that the system has a chance to load the segment. UB2 is the
procedure number.

**SCXGn \  **
Where n is in the range 1 to 8. This is a short form of CXG that calls
global procedure UB in the segment indicated by n.

**CXL \,\  **
Call eXternal Local procedure. UB1 is the segment number, UB2 the
procedure number.

**CXI \,\,\  **
Call eXternal Intermediary procedure. UB1 is the segment number, UB2 the
procedure number. DB is the number of nesting levels to be walked
backwards through the MSCWs to find the procedure.

**CFP \  **
Call Formal Procedure. This allows you to code a procedure call "on the
fly" rather than including it in the code stream. Three data words must
be placed on the stack prior to execution:
    |proc-num |
    |e-rec-ptr|
    |stat-link|
    |/////////|

Proc_num is the number of the procedure to be called, it will be placed
in the CURPROC register.
E_Rec_Ptr points to the environment record of the segment to be used.
Stat_link is the value to be placed in the MSSTAT field of the MSCW.

In other words, procedure \ in the segement indicated by
\ is called. I'm not sure what's the use of \, it may
be a typo in the manual...

**LSL \**

Load Static Link. DB indicates the number of static links to travel. A
pointer to the MSCW that is DB static links above the current one
(1=parent, 2=grandparent, etc) is placed on the stack. This comes handy
to prepare the stack for a CFP instruction.

**RPU  **
Return from ProcedUre. This is the instruction used to return from any
procedure call. The various fields in the MSCW are used to restore the
caller's environment:

- The E_Rec is restored from the value saved in MSENV (which may cause a
  segment fault if the segment isn't present any more and must be
  reloaded).
- MP is restored from MSDYN.
- CURPROC  is restored from MSPROC.
- Finally IPC can either be restored from MSIPC or set to the exit_ic
  value found upstream of the procedure in the code stream. Which of the
  two is performed depends whether MSPROC is positive or negative: if
  it's positive MSIPC is used, if it's negative exit_ic is used (in any
  case CURPROC will be positive). Thus you can cause the exit code to be
  executed by negating the MSPROC word in the MSCW: it's the first word
  after your local data.

####  Data access

Just as you can call global, local and intermediary procedures, you can
access global, local or intermediary-level variables. You can also
access global variables in another segment. For constants, you can
either access constants located in the constant pool of the current
segment or include a constant value whithin the code stream.

LDCB, LDCI, #SLDCSLDC, LDCN

Let's start with global variables:

**LAO \  **
Load glObal Address. Places the address of the global variable with
offset B (in the global MSCW pointed by BASE) on the stack.

**LDO \  **
LoaD glObal. Places the value of the global variable B on the stack.

**SLDOn  **
Where n is a number from 1 to 16. This is a short form of LDO, used to
load one of the first 16 global variable. The number of the variable is
part of the p-code, and thus does not require and extra byte (or two) in
the code stream.

**SRO \  **
StoRe glObal. Stores the word currently on the stack into global
variable B.
Now let's see local variables. They are created on stack when a
procedure is entered, between the calling parameters and the MSCW. They
are specifed by their offset with respect to the MSCW.

**LLA \  **
Load Local Address. Places on the stack the address of the local
variable with offset B in the data area of the current procedure (i.e.
on the stack, below the MSCW).

**SLLAn  **
Where n is a number from 1 to 8. This is a short form of LLA, that loads
the address of the first eight local variables. It is shorter than LLA
because the number of the variable is part of the p-code instead of
requiring an extra byte (or two).

**LDL \  **
LoaD Local. Places on the stack the value of the local variable with
offset B below the current MSCW.

**SLDLn  **
Where n is a number from 1 to 16. This is a short form of LDL, that
loads one of the first sixteen local variables.

**STL \  **
STore Local. The word at the top of the stack is poped and placed into
the variable with offset \ below the current MSCW.

**SSTLn  **
Where n is a number from 1 to 8. Short form of STL where the number of
the local variable to modify is part of the p-code.
Intermediary variables are local to a ancestor of the current procedure
and are therefore present on stack, below the MSCW of the ancestor. They
are accessed by walking up the chain of MSSTAT words in the MSCW of the
respective procedures.

**LDA \,\  **
Load intermeDiary Address. Places on the stack the address of variable B
below the MSCW found \ levels above the current one. DB=0 means the
current procedure (i.e. local variables), DB=1 means the lexical parent
in which the current procedure is nested, etc

**LOD \,\  **
LOad intermeDiate. Places on the stack the value of the variable with
offset B below the MSCW found \ levels above the current one .

**SLODn \  **
Where n is 1 or 2. Short form of LOD to access variables in the lexical
parent (SLOD1) or grandparent (SLOD2) of the current procedure.

**STR \,\  **
SToRe intermediate. DB indicates the number of static links to travel to
find the proper MSCW (0=current, 1=parent, etc). The variable with
offset \ in the target procedure will receive the word currently on
top of the stack.
It is also possible to access global variables in another segment.

**LAE \,\  **
Load Address Extended. The address of the global variable with offset B
in segment UB is placed on the stack.

**LDE \,\  **
LoaD Extended. Places the value of global variable B in segment UB on
the stack.

**STE  **
STore Extended. Pops the word on top of the stack and stores it in
global variable B in segment UB.
Constant values may be placed in a dedicated memory area called the code
pool, or included within the p-code stream.

**LCO \  **
Load Constant Offset. B is the offset of a constant in the constant pool
of the current segment (i.e. the constant "index"). The instructions
places the address of this constant on the stack, in the form of a
segment-relative offset.

**LDC \,\,\  **
LoaD Constant. B is the offset of a constant in the constant pool of the
current segment. The instructions pushes UB2 words on the stack,
starting from this offset. UB1 is the mode, if it is 2 and the current
segment is of the opposite byte sex than the host processor, bytes will
be swapped upon copy.

**LDCRL \  **
LoaD Constant ReaL. The real constant at offset B in the real sub-pool
(a specialized portion of the constant pool) is loaded on stack.
Depending on the format used by the code file, a real number uses up two
or four words on the stack.

With the following p-codes, the constant is part of the code:

**LDCB \  **
LoaD Constant Byte. Places the value \ on stack.

**LDCI \  **
LoaD Constant Integer. Places the value W on stack.

**LDCN  **
LoaD Constant Nil. Places the "nil" value on the stack. The value
depends on the microprocessor running the PME. With the TI-99/4A it is
\>0000.

**SLDCn  **
Short LoaD Constant. Where n is a number from 0 to 31. These p-codes
place a number from 0 (SLDC0) to 31 (SLDC31) on the stack.

####  Stack manipulation

Now that we have placed variables on the stack, we can manipulate it.

**DUP1  **
Duplicate 1 word. Pushes on top of the stack an extra copy of the word
at top of stack.

**DUPR  **
Duplicate real. Make an extra copy of the real at top of stack (two or
four words depending on the real number format).

**SWAP  **
Swaps the two words at the top of the stack.

#####  Real \<--\> integer conversions:

**FLT  **
FLoaT. Replaces the integer at top of stack by the corresponding real
number.

**TNC  **
TruNCate real. Assuming the stack contains a real number, it will be
replaced with the corresponding integer after truncation of the decimal
part. If the real is not in the range -32768 to 32767 a floating point
execution error is issued (and that's bad news!).

**RND  **
RouND. Replaces the real number on top of the stack with the
corresponding integer rounded to the nearest integer. If the number is
not in the range -32768 to +32767, an execution error occurs.

#####  Address \<--\> value conversions.

**LDB  **
LoaD Byte. Assuming a byte-pointer is at the top of the stack, it is
replaced with the value of this byte (in the form of a word whose high
byte is \>00).

**STB  **
STore Byte. Assuming the stack contains a byte pointer topped with a
byte value (i.e. a word with value 0 to 255), this value is assigned to
the pointed byte. The stack is cleared afterwards.

**IND \  **
INDex. Assuming the word at top of stack is the address of a record, it
is replaced by the value found at this address plus offset \. IND 0
is thus the equivalent of LDB for words.

**SINDn  **
Where n is a number from 0 to 7. This is a short from of IND, used to
access the first 8 words of a record.

**INC \  **
INCrement. Assuming the word on top of the stack is an address, it is
indexed by B (i.e. B is added to it).

**STO  **
STOre. Assuming the stack contains an address topped with an integer
value, this value is stored at the indicated address. Both are
subsequently removed from the stack.

**LDRL  **
LoaD Real. Assuming the top of the stack contains the address of a real
number, it is replaced with the value found at this address (two or four
words depending on the real number format).

**STRL  **
STore ReaL. Assuming the stack contains an address topped with a real
number, this number will be stored at the indicated address. Both the
number and the address are then removed from the stack.

**LDM \  **
LoaD Multiple. Assuming the stack contains an address, it is replaced
with a block of UB words found at this address. A stack fault is issued
if less than UB+20 words are available on the stack.

**STM \  **
STore Multiple. Assuming the stack contains an address, topped with a
block of UB words, these words will be stored at the indicated address.
The stack pointer is then adjusted to remove the block and the address
from the stack.

**LDP  **
LoaD Packed. Assuming a packed-field pointer is at the top of the stack,
it is replaced with the value of the field it points at. This value will
be in the form of a right-justified word, padded on the left with "0"
bits.

**STP  **
STore Packed. Assuming the stack contains a packed-field pointer, topped
with an integer value, the value it stored into the pointed field. Value
and pointer are removed from the stack.

**MOV \,\**

MOVe. The word on top of the stack is the address of a source block of
words. The word below it is the address of the destination. B is the
number of words to be copied from the source to the destination. If UB=0
the source is a variable, otherwise it is a constant and the word on top
of the stack is its offset in the current segment. If UB=2 and the
current segment has the opposite byte sex from the host processor, bytes
will be swapped upon copy.

####  Arithmetic and logic instructions

OK, now that we have data placed on stack, we can perform math on them.
Let's begin with integers:

**ABI  **
ABsolute value Integer. Takes the absolute value of the integer found on
top of the stack. Exception: -32768 stays as it is (since +32768 cannot
be represented with 16 bits).

**NGI  **
NeGate Integer. Negates the integer on top of the stack. -32768 remains
unchanged.

**DECI  **
DECrement Integer. Decrements by one the integer on top of the stack.
Wraps around from -32768 to +32767.

**INCI  **
INCrement Integer. Increments by one the integer on top of the stack.
Wraps around from +32767 to -32768.

**ADI  **
ADd Integer. Adds up the two integers on top of the stack and replaces
them with the result. Overflow and underflow wrap around to the opposite
sign.

**SBI  **
SuBstract Integer. Substract the integer on top of the stack from the
integer below it. Overflow and underflow wrap around.

**MPI  **
MultiPly Integer. Multiply the two integers on the top of the stack and
replace them with the result. The result is calculated as a 32-bit
number, but only the least significant 16 bits will appear on stack
(i.e. overflows result in crazy values).

**DVI  **
DiVide Integer. Divides the second integer from the top by the integer
on the top of the stack, and replaces them with the result. Truncation
is always toward zero. Divisions by zero cause an execution error.

**MODI  **
MODulo Integer. Adds or substract the word on top of the stack from the
word below it until the remainder is in the desired range. Replaces both
integers with the remainder. If the top word is zero an execution error
occurs, if it is negative the result is undefined but no error occurs.
And now, let's do the same for real numbers. Remember, depending on the
adopted format, a real number can be two or four words long:

**ABR  **
ABsolute Real. The real number on top of the stack is replaced with it
absolute value.

**NGR  **
NeGate Real. Negates the real number on top of the stack.

**ADR  **
ADd Real. Adds the two reals on top of stack and replaces them with the
result. Undeflow defaults to zero, overflow causes an execution error.

**SBR  **
SuBstract Real. Substract the real number on top of the stack from the
real number under it and replaces them with the result. Undeflow
defaults to zero, overflow causes an execution error.

**MPR  **
MultiPly Real. Multiplies the top reals on top of the stack and replaces
them with the result. Undeflow defaults to zero, overflow causes an
execution error.

**DVR  **
DiVide Real. Divides the second real from the top by the real above it
and replaces them with the result. Undeflow defaults to zero, overflow
causes an execution error.
Note that there is not p-code for modulo real. Let's conclude this
section with a few logical operations:

**LNOT  **
Inverts the word on top of the stack, bitwise (i.e. "0" bits are
replaced with "1" and conversely).

**BNOT  **
Same as LNOT, but only the least significant bit is kept. Thus the
result will always be \>0000 (false) or \>0001 (true) no matter how many
bits were set in the word.

**LAND  **
The two words on top of the stack are "anded" bitwise and replaced with
the result. Bitwise: 0 and 0 = 0, 0 and 1 = 0, 1 and 0 = 0, 1 and 1 = 1.

**LOR  **
The two words on top of the stack are "ored" bitwised and replaced with
the result. Bitwise: 0 or 0 = 0, 0 or 1 = 1, 1 or 0 = 1, 1 or 1 = 1.

NB. Sadly enough, there is no p-code for XOR operations (exclusive or).

####  Comparisons

The following p-code are used to make comparison between values placed
on the stack. The result of the comparison is generally a boolean word
(i.e. \>0001 for "true" and \>0000 for "false) that will **replace**
these values on the stack. The boolean can subsequently be used for a
conditional jump (see next section).

**EQUI  **
EQUal Integer. Places "true" on the stack if the two integers on top of
the stack are identical, "false" otherwise.

**NEQI  **
Not EQual Integer. Places "false" on the stack if the two integers on
top of the stack are identical, "true" otherwise.

**GEQI  **
Greater or EQual Integer. Places "true" on the stack if the second
integer from the top of the stack is greater than or equal to the
integer on top of the stack. This is a signed comparison, i.e. \>8002 (a
negative number) is smaller than \>7030.

**LEQI  **
Less Than or Equal Integer. Places "true" on the stack if the second
integer from the top of the stack is smaller than or equal to the
integer on top of the stack. This is a signed comparison, i.e. \>FFFF
(minus one) is smaller than \>0001.

**GEUSW  **
Greater or Equal UnSigned Word. Places "true" on the stack if the second
integer from the top of the stack is greater than or equal to the
integer on top of the stack. This is an unsigned comparison, i.e. \>8002
is greater than \>7030.

**LEUSW  **
Lower or Equal UnSigned Word. Places "true" on the stack if the second
integer from the top of the stack is smaller than or equal to the
integer on top of the stack. This is an unsigned comparison, i.e. \>FFFF
is greater than \>0001.

**EQREAL  **
EQual REALs. Places "true" on the stack if the two reals on top of the
stack are identical.

**GEREAL  **
Greater or Equal REAL. Places "true" on the stack if the second real
number from the top of the stack is greater than or equal to the real
number on top of the stack. This is an signed comparison, by definition.

**LEREAL  **
Less than or Equal REAL. Places "true" on the stack if the second real
number from the top of the stack is lower than or equal to the real
number on top of the stack. This is an signed comparison, by definition.
The following operations deal with array of bytes. The stack can contain
either the address of an array or its offset whithin the segment. Extra
parameters in the code stream decide which it is.

**EQBYT \,\,\  **
The two words on top of the stack should contain either the addresses of
two byte arrays, or their offset in the current segment. UB1 and UB2
determine whether the address (UBx = 0) or the offset (UBx \<\> 0) is
used: UB1 refers to the word on top of the stack, UB2 to the word below.
B is the size of the array. The arrays are compared byte per byte in the
natural order. If they are identical "true" is placed on the stack.

**GEBYT \,\,\  **
Greater or Equal BYTe array. Same as above, but the comparison stops as
soon as a mismatch is encountered. It returns "true" if the mismatched
byte in the "bottom" array (pointed by the second word from the top of
the stack) is greater than a byte in the "top" array, or if both arrays
are identical.

**LEBYT \,\,\  **
Less or Equal BYTe array. Same as above, but the comparison returns
"true" if the first mismatched byte in the bottom array is lower than
the corresponding byte in the top array (or if there is no mismatch).
The following p-codes deal with strings. A string is essentially the
same than an array of bytes, excepts that it begins with a length byte.
Thus string comparisons begin with the second byte. If both strings are
identical (upto the end of the shorter one), the size bytes are
compared.

**EQSTR \,\  **
EQual STRing. The two words on top of the stack should contain either
the addresses of two strings, or their offset in the current segment.
UB1 and UB2 determine wether the address (UBx = 0) or the offset (UBx
\<\> 0) is used: UB1 refers to the word on top of the stack, UB2 to the
word below. The strings are compared byte per byte from the second byte
to the end of the shorter string. If they are identical, their first
bytes (the size) are compared. If these are identical, "true" is placed
on the stack.

**GESTR \,\  **
Greater or Equal STRing. Same as above, but the comparison stops as soon
as a mismatch is encountered. It returns "true" if the mismatched byte
in the "bottom" string (pointed by the second word from the top of the
stack) is greater than a byte in the "top" string or if both arrays are
identical.

**LESTR \,\  **
Less or Equal String. Same as above, but the comparison returns "true"
if the first mismatched byte in the string is lower than the
corresponding byte in the top string (or if there is no mismatch).
The following p-codes deal with packed sets, and don't ask me why they
are abbreviated PWR (packed words? power?). A set consists in upto 255
words to be accessed on a bitwise basis, preceded with a length word
(that contains the number of words in the set).

**EQPWR  **
The two sets on top of the stack are compared upto the end of the
shorter one. "True" is pushed on the stack of no mismatch is found (even
if the sets are not the same size).

**GEPWR  **
"True" is pushed on the stack if the set on top of the stack is a subset
of the set below is. Which means that all the "1" bits in the top set
are also "1" is the bottom set (I think).

**LEPWR  **
"True" is pushed on the stack if the set on top of the stack is a
superset of the set below it, "false" is pused otherwise.

####  Flow control

Now that we have placed a boolean on top of the stack with a comparison,
we can use the following p-code to perform conditional jumps. The
address where to jump to is expressed as an offset in bytes, relative to
the next instructio. This nifty feature allows a p-code program to be
shifted in memory during execution without messing up all the addresses.

**TJP \  **
True JumP. Jumps by SB bytes if the boolean word on top of the stack is
"true" (i.e. its last bit is 1).

**FJP \  **
False JumP. Jumps by SB bytes (relative to the next instruction) if the
boolean word on top of the stack is "false" (i.e. its rightmost bit is
0).

**FJPL \  **
False JumP Long. Same as above, but the offset is expressed as a word,
which means that you can jump futher than away than -128 to +127 bytes.
(And why on hearth is there no "true jump long" ?).

**UJP \  **
Unconditional JumP. Jumps by SB bytes (-128 to +127 relative to the next
instruction). The stack is ignored and not affected.

**UJPL \  **
Unconditional JumP Long. Same as above, but in the ranger -32768 to
32767 bytes relative to the next instruction.
Alternatively, we can compare integers and perform the jump with a
single instruction:

**EFJ \  **
Equal False Jump. Jumps by SB bytes (relative to the next instruction)
if the two integers on top of the stack are different.

**NFJ \  **
Not equal False Jump. These guys like double negations! Jumps by SB
bytes if it is false that the two words on top of the stack are not
equal. In layman's terms this means that the jump if taken if the words
are identical.
Here is a more sophisticated form of jump:

**XJP \  **
case JumP (what does the X stand for in "case"?). B is the offset of a
jump table in the constant pool. The integer on top of the stack should
be an index whithin that table. The corresponding word in the table is
used as a jump offset, relative to the next instruction. A jump table
begins with two words that indicate the minimum and the maximum index,
if the integer on top of the stack is not in this range, no jump is
taken.

Example of a jump table created by the Pascal instruction:

    case I of                       >0002  Minimum index (table begins at 2)
     2: xxx ;                       >0005  Maximum index (table ends at 5)
     5: xxx ;                       >FFFA  Jump offset for I=2
     3:xxx;                         >FFFE  Jump offset for I=3
    end;                            >0000  No jump for 4 (as it does not exist)
                                    >FFFC  Jump offset for I=5
Two more p-codes that can fall in the category of flow controlling
instructions:

**BPT  **
BreakPoinT. A breakpoint execution error (number 16) is issued
unconditionally. This will result in entering the debugger, if it is
running. NB The debugger is not implemented on the TI-99/4A (rats!).

**CHK  **
CHecK subrange bounds. The stack contains three integers, from top to
bottom: the upper limit, the lower limit and the test value. If the test
value does not fall whithing the specified range (as judged by signed
comparisons) a value range execution error is issued. After the
instruction, the two limits are removed but the test value remains on
the stack.

    Examples:
    44           32              44
    12          -21              12
    18            0            1233
    OK           OK            Error

####   Arrays and strings

Arrays can contain elements of any size, but all elements in the array
must have the same size. If the size is not a multiple of a byte (e.g.
if it is 10 bits, or 3 bits) elements can be squeezed together,
regardless of byte boundaries. This is called a packed array, just like
in Pascal.

A string is an array of characters terminated with a nul byte (\>00).

**ASTR \,\**

Assign STRing. Copies the string whose address is at the top of the
stack, into another string whose address is just below on the stack. UB2
is the size of the string. UB1 determines whether the source string is a
variable (UB1=0) or a constant (in which case the word at top of stack
is an offset in the constant pool). If the source string happens to be
greater than UB2 bytes, a "string overflow" execution error occurs.

**CAP \**

Copy Array Parameters. Pretty much like CSP, but the parameter copied is
a packed array of characters (i.e. is not necessarily nul-terminated).
Thus, there can be no check for size, and \ characters are always
copied to the buffer.

**CSP \**

Copy String Parameter. This instruction is usefull to copy a function
parameter into a local variable:

    MyFunction( S1: string);
    {
        var S2: string;
    }

For obvious reasons, string parameters are passed by pointers. The word
on top of the stacks is the address of a two-word parameter descriptor.
If the bottom word of the descriptor is NIL, the second word is the
address of the string parameter (S1 in the above example).
Alternatively, the bottom word can be a pointer to an E_Rec (environment
record), in which case the second word is the offset of the string in
the segment corresponding to this E_Rec. If this segment is not
resident, a segment fault occurs.

The second word from the top of the stack is the address of a local
string buffer (S2 in the exemple) into which the string parameter will
be copied. UB indicates the size of this buffer. If the parameter string
happens to be larger, a "string overflow" execution error occurs.

**CSTR**

Check STRing index. The integer on top of the stack is an index inside a
string whose address is in the word below it. If this index is less than
1, or greater than the dynamic length of the string, a "value range"
execution error occurs.

**IXA \**

IndeX Array. The integer on top of the stack in an index inside an array
whose address is in the word below it. B is the size in words of an
array element. These two words are removed from the stack and replaced
with the address of the corresponding element. The algorithm is: Result
= Address + (Index \* \ \* atype), where atype is 1 for
word-addressed computers and 2 for byte-addressed computers.

**IXP \,\**

IndeX Packed array. The integer on top of the stack in an index inside a
packed array (i.e. an array of bit-fields) whose address is in the word
below it. UB1 is the number of elements per word, UB2 is the size in
bits of an element . These two words are removed from the stack and
replaced with the corresponding element.

####  Sets

A set consits in 0 to 255 words of bit flags, preceded by a word that
indicates the total number of words in the set. Bits are numbered from
0, starting with the rightmost bit of the first word. A bit set to "1"
indicates that the corresponding number is present in the set, a "0"
that it is not a member of the set.

Example:

The set \[0,3,5,7\] is represented as:

    00000000 00000001  00000000 10101001
    Size in words (=1) Bits:    7 5 3  0
**ADJ \**

The set on top of the stack is stripped of its length word, then
expanded or compressed until is UB words in length. Compression removes
the high words of the set (even if they do not contain zero!), expansion
adds zero-words on top of the stack.

If, after the operation, less than 20 words remain free for the stack to
grow, a stack fault is issued.

**SRS**

SubRangeSet. Two integers in the range 0 to 4097 should be on top of the
stack (if not, a "value range" execution error occurs). If the top word
is the smallest, the empty set is placed on the stack. Otherwise, a set
is placed on the stack in which all bits between these two integers
(included) are set as "1".

If less than 20 words remain available on the stack after execution, a
stack fault is issued.

**INN**

INcluded iN. Checks for set membership. The word on top of the stack is
the index of a bit in the set below it. If this bit is "1", the stack
will contain "true" after the operation, otherwise it will contain
"false". Note that bit numbering starts from zero, with the rightmost
bit of a word.

**DIF**

DIFference. Removes the two sets on top of the stack and replaces them a
set corresponding to their bitwise difference. The difference is
computed as: Set1 AND NOT Set2 ( 0 DIF  0 = 0, 0 DIF 1 = 0, 1 DIF 0 = 1,
1 DIF 1= 0).

**INT**

INTersection. The intersection of the two sets on top of the stack is
computed as Set1 AND Set2 and the resulting set replaces these two on
the stack.

**UNI**

UNIon. The two sets on top of the stack are replaced with their bitwise
union. The union is computed as: Set1 OR Set2.

####  Multitasking

**WAIT**

Waits for a semaphore. The word on top of the stack is the address of
this semaphore. If the semaphore value is greater than zero it is
decremented and execution continues. If it is zero, the current tasks
must wait for the semaphore to become available. To this end, the
current TIB is placed in the semaphore's queue (and Hang_Ptr in the TIB
will point to the semaphore). Then the p-system switches to another
task.

**SIGNAL**

Signals a semaphore. The word on top of the stack is the address of this
semaphore. If the semaphore is less than zero (or if its queue is empty)
it is incremented and execution continues. Otherwise, the first task
waiting for this semaphore is removed from the semaphore queue and
placed in the queue of tasks ready to run (and the Hang_Ptr in its TIB
is set to NIL). If this task has a higher priority than the current one,
the p-system switches to it.

####  Others p-codes

These do not fit in any of the above categories.

**LPR**

Load Processor Register. The word on top of the stack is a register
number. It will be replaced on the stack by the content of this
register. The register numbers are:

-3: READYQ
-2: EVEC
-1: CURTASK
0: Wait-Q
1: Prior \| Flags
2: SP_Low
3: SP_Up
4: SP
5: MP
6: (reserved)
7: IPC
8: Env
9: Proc_Num \| TIB_IOresult
10: Hang_Ptr
11: M_Depend

**SPR**

Store Processor Registers. The word on top of the stack is stored in the
register whose number is in the word below it. See LPR for the register
numbers. If the number is 0 or greater, all registers are first saved in
the TIB, then the adequate value is placed in the TIB, finally all
registers are restored from the TIB.

**NAT**

Enter NATive code. Starts executing assembly language right after this
p-code. It may be necessary to increment IPC to a word boundary on
word-oriented processors (such as the TMS9900).

**NAT-INFO \**

Same as NAT, but IPC will be incremented by B before entering assembly
language (i.e. it acts as a jump). If B=0, the first execution begins
with the byte right after B in the code stream. This allows place
infomation for the native-code generators uptream of a stretch of
assembly language.

**NOP**

No OPeration. Execution continues.

**RESERVEn**

Where n is a number from 1 to 6. These p-codes are reserved for internal
used by compilers. Trying to execute them generates an "unimplemented"
execution error.

###   Standard procedures

A number of standard procedures have been included whithin the PME,
either for speed, or because they contain some assembly language. These
procedures should be called with CXG or SCXG1. Most of them expect
parameters on the stack, and some return a result on the stack. In any
case, the initial parameters will be removed from the stack by the
procedure itself. The standard procedures differ from user-written
procedures in that they do not return with RPU. In a sense, they can be
viewed as extra p-codes...

For consistency I listed them in their Pascal form, but you can easily
convert the declarations into a stack description. For instance, with
the following procedure

`DUMMY(UNIT: integer, PTR:addr): Boolean `

the stack would look like this

     ______ <
    | addr |
    | int  |                <
    |  0   |        | bool |
    |//////|        |//////|
     Before          After

To call a procedure from assembly or p-code (when using a p-code
assembler), you should push zero on the stack if the function is
returning a value, then push the parameters in the order they appear in
the Pascal declaration, and finally call the procedure. When it returns,
make sure you pop the return value from the stack, if there is one.

POOL: addr
OFFSET: integer
DEST: byte_ptr
LEN: integer
DEST: byte_ptr
LEN: integer
BUFFER: byte_ptr
LEN: integer
BLOCK: integer
CTRL: integer
BUFFER: byte_ptr
LEN: integer
BLOCK: integer
CTRL: integer
LOWORD: addr
LEN: byte_ptr
CHAR: integer
EXP: Boolean
CHAR: byte
SOURCE :byte_ptr
MASK: word
POOL: addr
OFFSET: integer
NBYTES: integer
POOL: addr
OFFSET: integer
NBYTES: integer
OFFSET: integer
NWORDS: integer
VECTOR: integer
STAT_REC: addr
CTRL: integer
BUFFER: addr
FOUNDP: addr
TARGET: addr

####  Unit-related procedures

All these procedures place status information in IORESULT, upon return.
See [below](#ioresult) for details.

**UNITCLEAR(UNIT: integer)**

UNIT is the unit number. The procedure initializes the unit and returns
its status into the p-machine register IORESULT.
**UNITSTATUS(UNIT: integer; STAT_REC:addr;
CTRL:integer)**

UNIT is the unit number.
STAT_REC is the address of a status record.
If CTRL is 0 status is written, if CTRL is 1 status is read.
**UNITREAD(UNIT:integer; BUFFER:byte_ptr;
LEN:integer; BLOCK:integer; CTRL:integer)**

This procedures reads a number of bytes from a unit into a buffer.

UNIT is the unit number
BUFFER is a pointer to the destination buffer
LEN is the numbe of bytes to read
BLOCK is the number of the desired data block within the unit
CTRL selects the input mode (see BIOS for details)
**UNITWRITE(UNIT:integer; BUFFER:byte_ptr;
LEN:integer; BLOCK:integer; CTRL:integer)**

This procedures writes a number of bytes from a buffer into a unit.

UNIT is the unit number
BUFFER is a pointer to the source buffer
LEN is the numbe of bytes to write
BLOCK is the number of the target data block within the unit
CTRL selects the input mode (see BIOS for details)
**UNITWAIT(UNIT: integer)**

This procedure waits until all I/O on this unit is completed.
**UNITBUSY(UNIT:integer) :boolean**

This procedure returns TRUE if there are any pending I/O operations in
the unit, and FALSE otherwise.
**IORESULT(): integer**

This procedure returns the value of the IORESULT register.
**IOCHECK()**

This procedure verifies that the IORESULT register contains zero. If
not, an I/O execution error is issued.

####  String-related procedures

**MOVELEFT(SOURCE:byte_ptr; DEST:byte_ptr;
LEN:integer)**

This procedure moves a number of bytes from the source string to the
destination string, starting from the left (low-order) byte. If LEN is 0
or less, nothing happens.
**MOVERIGHT(SOURCE:byte_ptr; DEST:byte_ptr;
LEN:integer)**

This procedures moves a number of bytes from the source string to the
destination string, starting from the right (high order) byte. If LEN is
0 or less, nothing happens.
**FILLCHAR(DEST:byte_ptr; LEN:byte_ptr;
CHAR:integer)**

The destination string will be filled with the LEN copies of character
CHAR. If LEN is 0 or less, nothing happens.
**SCAN(LEN:integer; EXP:Boolean; CHAR:byte;
SOURCE:byte_ptr; MASK:word): integer**

This procedure looks for a character in a string.

If EXP is 0, the procedure scans the string SOURCE for LEN bytes, until
it finds the byte CHAR.
If EXP is 1, the procedure scans SOURCE for a byte different from
CHAR.
LEN is the maximum number of bytes to scan. If it is negative, scanning
proceeds from right to left.
MASK is not used.

The function returns the offset of CHAR within SOURCE (or LEN if no
match was found).

####  Code pool management procedures

**RELOCSEG(EREC:E_Rec)**

Relocates the segment pointed at by the environment record EREC. All
types of relocations are performed.
**MOVESEG(SIB:addr; POOL:addr;
OFFSET:integer)**

SIB is a pointer to a segment information block that should contain the
address where the segment should be moved.
OFFSET is the offset whithin the code pool of the segment to be moved.
POOL points to a code pool descriptor, the first two words of which are
a pointer to the base of the code pool.

The segment found at the specified offset whithin the code pool will be
moved at the location specified in the SIB and relocated. Only
segment-relative relocation is performed (as other types are not
affected by the move).

**GETPOOLBYTES(DEST:addr; POOL:addr;
OFFSET:integer; NBYTES:integer)**

This procedure copies a number of bytes from the code pool into a
buffer.

DEST is the address of the destination buffer.
POOL is the address of a code pool descriptor, the first two words of
which point to base of the pool.
OFFSET is the offset of the first byte to transfer, relative to the base
of the pool.
NBYTES is the number of bytes to transfer.

**PUTPOOLBYTES(DEST:addr; POOL:addr;
OFFSET:integer; NBYTES:integer)**

This procedures copies NBYTES bytes from buffer SOURCE into a code pool
described in POOL, starting at offset OFFSET. It's the opposite of the
above.

**FLIPSEGBYTES(addr:EREC, int:OFFSET,
int:NWORDS:int)**

This procedures flips the bytes within a number of words of a segment.

EREC is the address of an environment record describing the segment.
OFFSET is the word offset where byte flipping should start.
NWORDS is the number of words whose bytes should be flipped.

**READSEG(EREC:addr): integer**

This procedures reads a segment into memory.

EREC is the address of an environment record describing the segment.
Inside the E_Rec is a pointer to the SIB (segment information block)
that contains the address where the segment should be read.

The procedure returns the contents of the IORESULT register.

####  Concurency procedures

**ATTACH(SEM:addr, VECTOR:integer)**

This procedure associates a semaphore with a p-machine event. The
semaphore will be signaled each time the desired event occurs.

SEM is the address of a semaphore. If it is NIL, the event be detached
from any existing semaphore.
VECTOR is the number of a p-machine event vector. It should be in the
range 0 to 63, otherwise nothing happens. I have no idea what the events
are...

**QUIET()**

This procedure disables all p-machine events such that no attached
semaphore is signalled until ENABLE is called.

**ENABLE()**

This procedure re-enables the p-machine events that were disabled by
QUIET.

####  Miscellaneous procedures

**TIME(HIWORD:addr; LOWORD:addr)**

Saves the value of the p-system clock into the indicated words. This
clock is a 32-bit number, incremented 60 times per seconds.

**POWEROFTEN(POWER:integer): real**

Returns a real number equals to 10 to the power of POWER. If POWER is
negative or too big, a floating point execution error is issued.

####  Compiler usage procedures

These two procedures are used by the Pascal compiler. They have nothing
to do in the PME, and were only placed here to speed up compilation.

**TREESEARCH(ROOT:addr; FOUNDP:addr;
TARGET:addr): integer**

This procedure searches the symbol table tree for a target string. Each
tree node is expected to have the following structure:

     RECORD
      Name: PACKED ARRAY[0..7] of CHAR;
      Right_link: pointer;
      Left_link: pointer;
     END

ROOT is the address of the (sub)tree in the symbol table.
TARGET is the address of the string to look for, a packed array of 8
characters.
FOUNDP is the address where to put the result of the search, i.e. where
the string was found.

If the target string was found, the function returns 0 and places a
pointer to the leaf node that contains it into FOUNDP.
If the target was not found, FOUNP will point to the last node that was
scanned and the function return 1 of the target should be placed to the
right of this node, -1 if it should go to the left.

**IDSEARCH(SYMREC:addr; BUFFER:addr)**

This procedure scans a line of text for a Pascal keyword.

BUFFER is the address of the buffer to scan
SYMREC is the address of a record with the following structure:

     RECORD
      SYMCURSOR: integer;
      SY: integer;
      OP: integer;
      ID: PACKED ARRAY[0..7] OF CHARS;
     END

The procedure scans BUFFER, starting at position SYMCURSOR, looking for
an identifier. An identifier is a string containing only letters, digits
and underscore, that begins with a letter. Upon return the first 8
characters of the identifier will be placed in ID, using padding spaces
if needed, and SYMCURSOR will be updated to point after the identifier.
Finally, the function looks up a table of reserved words and placed the
corresponding values in SY and OP.

Here are the reserved words recognised by the procedure, and the return
values. The table is laid out in two columns for convenience.

|           |     |     |          |     |     |
|-----------|-----|-----|----------|-----|-----|
| ID        | SY  | OP  | ID       | SY  | OP  |
| AND       | 39  | 2   | NOT      | 38  | 15  |
| ARRAY     | 44  | 15  | OF       | 11  | 15  |
| BEGIN     | 19  | 15  | OR       | 40  | 7   |
| CASE      | 21  | 15  | PACKED   | 43  | 15  |
| CONST     | 28  | 15  | PROCEDUR | 31  | 15  |
| DIV       | 39  | 3   | PROCESS  | 56  | 15  |
| DO        | 6   | 15  | PROGRAM  | 33  | 15  |
| DOWNTO    | 8   | 15  | REPEAT   | 22  | 15  |
| ELSE      | 3   | 15  | RECORD   | 45  | 15  |
| END       | 9   | 15  | SET      | 42  | 15  |
| EXTERNAL  | 53  | 15  | SEGMENT  | 33  | 15  |
| FILE      | 46  | 15  | SEPARATE | 54  | 15  |
| FOR       | 2   | 15  | THEN     | 12  | 15  |
| FORWARD   | 34  | 15  | TO       | 7   | 15  |
| FUNCTION  | 32  | 15  | TYPE     | 29  | 15  |
| GOTO      | 26  | 15  | UNIT     | 50  | 15  |
| IF        | 20  | 15  | UNTIL    | 10  | 15  |
| IMPLEMENT | 52  | 15  | USES     | 49  | 15  |
| IN        | 41  | 14  | VAR      | 30  | 15  |
| INTERFAC  | 51  | 15  | WHILE    | 23  | 15  |
| LABEL     | 27  | 15  | WITH     | 25  | 15  |
| MOD       | 39  | 4   | unknown  | 0   | 15  |

###   Faults and execution errors

Two types of problems can occur while executing p-codes: faults and
execution errors. A fault is a benigne condition, that requires the
assistance of the p-system to be fixed. After this is done, execution
can continue. By contrast, an execution error is generally fatal and
will result in terminating the program that caused it and reseting the
p-system.

#### Faults

The PME only generates two types of faults: segment faults and stack
faults.

**Segment faults** occurs when a p-code tries to access a segment that
is not (yet) in memory. These p-codes are: CAP, CSP, CXL, SCXGn, CXG,
CXI, CFP, RPU, SIGNAL and WAIT. The p-machine signals the semaphore
Real_Sem to allert the p-system and the fault condition is handled by
the high priority task Fault_Handler, which is part of the KERNEL unit.
Typically, this will result in loading the required segment into memory.

**Stack faults** occurs when there is not enough room on the stack. Only
p-codes that place multiple words on the stack check it for room (except
for real numbers). These p-codes are: ADJ, CFP, CGP, SCIPn, CIP, CLP,
CXL, SCXGn, CXG, LDC, and LDM. Here, Fault_Handler will try to compact
the memory so as to free some more room for the stack.

Before signalling Real_Sem, the PME saves some values into the SYSCOM
area for Fault_Handler to use. These are:

    Offset Name         Usage
    14     Real_Sem     Semaphore that Fault_Handler is waiting for
    18     Fault_TIB    Task info block of the faulting task
    20     Fault_EREC   E_Rec of the segment to be loaded (current E_rec for stack faults)
    22     Fault_Words  Number of words needed on stack (0 with segment faults)
    24     Fault_Type   >80 = PME segment fault, >81 = PME stack fault

Once the problem has been taken care of, control is returned to the
p-machine emulator and the p-code that caused the fault is executed
again.

####  Execution errors

When an execution error occurs, the PME calls the procedure Exec_Error,
which is also part of the Kernel. This call is performed with CXG 1,2
after placing two zeros on the stack, followed by the error number.

Normally, Exec_Error will not return but rather reset the p-system,
although it is possible to cause it to return (for instance from a
debugger program).

The possible execution errors are:


CXL, SCXGn, CXG, CXI
is 0 (probably because it was't linked).
or too large to be poped from stack.
RND, ASBR, NGR, ADR, SBR,
MPR, DVR, EQREAL, LEREAL,
GEREAL, POWEROFTEN
cannot be expressed as a float).
(this should never happen).

###   Data representations

The p-system was designed to run on any computer system, provided the
proper PME is written. However, different computers handle numbers in a
different way. Everybody agrees on the definition of a byte, but things
become more complicated when it comes to larger numbers.

#### Words

Almost everybody agrees that a word is a 16-bit value, i.e. is made of
two bytes. However, there are two conventions on how to order the bytes
within a word. For instance, suppose you want to place the number \>1234
in memory, at address \>A000-A001. There are two ways to do it:

    +-----+             +-----+
    | >34 |             | >12 |
    +-----+ >A001       +-----+  >A001
    | >12 |             | >34 |
    +-----+ >A000       +-----+  >A000
    Big-endian          Little-endian
    TI, Motorola        Intel, PCs

With the first convention, the bytes are stored in "natural" order, i.e.
the most significant byte comes first, when moving ahead in memory. With
the second, the most significant byte goes into the highest address. TI
uses the first convention, so does Motorola (and consequently Apple
computers), whereas Intel uses the second and so do all PC clones.

As p-code is to be ported on any machine, a considerable effort was put
into it so that it is independent of this so-called "byte sex". Since
all p-codes are bytes, that part was easy. But trouble began with
constants and addresses, which are word-oriented. The following solution
was adopted: 1) The p-system always uses the byte sex of the machine it
runs onto. 2) Each segment in a code file contains a flag indicating the
byte sex used by the compiler that created it. If it does not match the
current byte sex, the PME will flip bytes in the relevant portions of
this segment.

####  Long integers

Again, the definition of a long integer can vary from compiler to
compiler. Therefore, in the PME, integers can have any size from 2 to 10
words (thats 160 bits, about 36 decimal digits!). When it's on the
stack, an integer is always preceded by a length word, i.e. a number
from 2 to 10, indicating the number of words that follow.

    +-----+
    |>0003|  Size word
    +-----+
    |>1234|
    +-----+
    |>6635|  3-word Long
    +-----+
    |>8701|
    +-----+
    |/////|

However, when a long integer is assigned to a variable, or stored on
disk (e.g. as a constant embedded in a program), the length word is
stripped and the long integer is forced into the default "long" size on
the current processor. If this conversion is not possible, an "integer
overflow" execution error is issued.

To avoid that kind of problem as much as possible, the routine CVT
should be used to incorporate long constants into a program. It will
convert an integer (or any combination thereof) into a long integer
constant of the size required by the target processor. Thefore the code
file will not contain any long integer, which makes it fully portable.
Long integers will be created on the fly, when the program is run on the
target machine.

Examples:

    To code      Use
         12      CVT(12)
     225543      CVT(22554)*CVT(10) + CVT(3)
    -873342     -(CVT(8733)*CVT(1000) + CVT(442))

As you can see, all the parameters of CVT are integers (-32768 to
+32767) and can be coded as words. CVT does the dirty work in converting
them to a long integer of the appropriate size.

The CVT routine actually calls a package of routines that perform long
integer math. These are part of a Pascal unit called LONGOPS, that
contains three procedures:

FREADDEC reads a long integer
FWRITEDEC writes a long integer
DECOPS performs several math operations.

A peculiarity of DECOPS is that it takes a variable number of
parameters, depending on the operation to be performed. This is not
correct Pascal, but it's sure more convenient this way! This is possible
because DECOPS is actually an assembly routine, embedded into the Pascal
unit. When calling DECOPS, the required parameters are pushed on the
stack, followed by OP, the number of the operation to be executed (and
by the return address, obviously).

Valid DECOPS operations are:


LINT: long
If not possible: causes integer overflow.
LINT2: long
Can cause overflow if the result is more than 10 words.
LINT2: long
Can cause overflow if the result is more than 10 words.
LINT2: long
Can cause overflow if the result is more than 10 words.
LINT2: long
Can cause overflow or divide-by-zero errors.
ADDR: word
LINT: long
SIZE is the maximum number of characters in the string, if it is
exceeded a string-overflow error occurs.
INT: word
RESULT: long
on the top of the stack.
LINT1: long
LINT2: long
If TYPE = 1, returns LINT1 &lt;= LINT2
If TYPE = 2, returns LINT1 &gt;= LINT2
If TYPE = 3, returns LINT1 &gt; LINT2
If TYPE = 4, returns LINT1 &lt;&gt; LINT2
If TYPE = 5, returns LINT1 = LINT2
Can cause overflow if the result doesn't fit in 16 bits.

####   Real numbers

Encoding real numbers in floating point format so they can be ahndled by
a digital computer is quite a challenge. Not surprisingly, different
people came up with widely different schemes. (Texas Instruments did a
pretty good job with its 8-byte radix 100 format). But p-code is
supposed to run on any machine, so how can it standardize real numbers?

Well, actually it doesn't! Real numbers calculations are always
performed in the format used by the machine on which the program runs.
The appropriate floating point math routines are coded inside the PME.

However, there is still a problem when it comes to program files as
there may be a need to include real constants within a program. To solve
this problem, the p-system defines two "canonical" formats for real
constants. Whithin a program file, all real-number constants are grouped
together in a so-called "constant sub-pool". When loading the program
into memory, the p-system converts canonical real numbers into the
format required by the target computer.

For some reasons, there are two canonical formats in p-code: 3-words
reals, and 3-to-6-words reals. They are unfortunately incompatible and
all compilation units in a p-code program must use the same format.
There is a word in each segment that indicates the real size being used
by this segment.

The 3-words format is quite simple:

The first word is a signed integer containing the exponent (-32768 to
+32767)

The second word is a signed integer from -9999 to +9999 containing the
first 4 mantissa digits, including its sign.

The third word is an integer from 1 to +9999 containing the last 4
mantissa digits, left justified. If this word is negative, it will be
ignored.

The decimal point is assumed to be at the end of the last word in use
(i.e. the second word if the third is negative, otherwise the third
word).

The digits in the last mantissa word in use are left justified, i.e. the
word is padded with zeros on the right. (e.g. 11 is coded at 1100, which
is \>044C).

The second format, is just an extension of the above one: extra words
can be added upto a total of 5 mantissa words, to accomodate more
decimal figures. Just as above, a negative number in words 3 to 5 serves
as a terminator and all the following words will be omited. The decimal
point is always assumed to be at the end of the last word in use, and
the digits in this word will be left justified.

Examples:

    Real number         Exponent    Mantissa
    1234.0                  0        1234   -1
    -1234.0                 0       -1234   -1
    12345678.0              0        1234   5678
    12345678.0              0        1234   5678   -1
    1.234                  -3        1234   -1
    12.0                   -2        1200   -1
    12345678.2233          -4        1234   5678   2233   -1
    111122223333.4444555   -8        1111   2222   3333   44444   5550
    1.0 E+70               73        1000   -1

Note the two different ways to code number 12345678: the first one is
the 3-words format, the next is the 3-to-6 words format (in this case,
it's 4-word long).

------------------------------------------------------------------------

The RSP

RSP stands for Run-time Service Package. It is the part of the PME that
does not deal directly with interpreting p-codes, but rather handles
various run-time chores. An important part of it is the RSP/IO that
deals with I/O operations and provides a standardized interface with the
underlying BIOS.

The RSP/IO implements the concept of "device units". A device is a
peripheral that should be accessed in a standard manner, through a few
predefined operations. Note how this resembles the concept of DSRs used
by TI.

[Devices
](#device%20numbers)**Procedures  **
[\_Ioresult
](#ioresult)[\_Unitread
](#unitread)[\_Unitwrite
](#unitwrite)[\_Unitclear
](#unitclear)[\_Unitstatus
](#unitstatus)[\_Unitbusy
](#unitbusy)[\_Unitwait
](#unitwait)[Special characters
](#special%20characters)[Subsidiary volumes](#Subsidiary%20volumes)

###  Device numbers

Each device is refered in the p-system with a unique number, from 0 to
255. Some device numbers are pre-assigned by the system:

    0  (reserved)
    1:  CONSOLE
    2:  SYSTERM
    3:  (reserved)
    4:  DSK1
    5:  DSK2
    6:  PRINTER
    7:  REMIN
    8:  REMOUT
    9:  DSK3
    10: DSK4
    11: DSK5
    12: DSK6
    13-127: additional disks, subsidiary volumes, user-defined serial devices
    128-255: user-defined devices.

Extra units defined in the TI-99/4A p-system implementation:

    14: OS      Card GROMs, contain the system files for the main menu.
    31: TAPE    Cassette tape recorder.
    32: TP      Thermal printer.

###  Procedures

#### Ioresult

Most of the procedures describes below will cause a completion code to
be placed in the SYSCOM area, which is an area of memory shared by the
p-system and the PME. You can retrieve this completion code with the
procedure IORESULT. The codes are the following:

0: No error
1: Bad block, CRC error
2: Bad device number
3: Illegal I/O request
4: Data-com timeout
5: Volume went off-line
6: File is no longer in directory
7: Illegal filename
8: No room on volume
9: Volume not found
10: File not found
11: Duplicate file
12: Attempt to open an already opened file
13: Attempt to close a file that isn't open
14: Bad format on reading a real or an integer
15: Ring buffer overflow
16: Disk is write-protected
17: Illegal block number
18: Illegal buffer address
19: Bad text file size
20-127: (reserved)

####  Unitread

This procedures reads a given number of bytes from a device into a
memory buffer.

In Pascal, its declaration would look like this:

    PROCEDURE UNITREAD(UNITNUMBER: INTEGER;
                       VAR DATA_AREA: PACKED ARRAY[1...NBYTES-1] OF [0...255];
                       NBYTES: INTEGER;
                       [LBLOCK: INTEGER;]
                       [CTRL: INTEGER;] );

Ok, this is not correct syntax, since Pascal does not allow optional
parameters, nor variable-length arrays, but you get the idea. The
parameters are the following:

**`UNITNUMBER`**: the number of the device unit as defined
[above](#device%20numbers).
**`DATA_AREA`**: A pointer to the buffer where data is to be stored.
This buffer is an array of bytes of length `NBYTES`. The pointer is
actually made up of two words: a word base and a byte offset.
Byte-oriented machines just add the two numbers to get the buffer
address. Word-oriented machines compute the buffer address by indexing
byte-wise from the word base.
**`NBYTES`**: Number of bytes to transfer.
**`LBLOCK`**: Logical block. This is an optional parameter used with
disk devices. Each disk is considered as an array of logical blocks each
512 bytes in length (If this is not the real format of the disk, the
BIOS must do the conversion, so that it appears so to the RSP). If this
parameter is not specified, it will be passed as 0.
**`CTRL`**: Optional control word parameter, will be 0 if ommited. Out
of 16 bits, only 7 are currently defined. Note that a given unit may use
only a few of them.

    UUUr rrrr rrrr CSPA

- U = user defined control-bits.
  r = r eserved bits.
  C = NOCLRF. If this bit is 0, a LF (ascii \>0A) will be automatically
  appended to each CR (ascii \>0D).
  S = NOSPEC. If this bit is 1, special characters handling is
  disabled.
  P = PHYSSECT. If this bit is 1, the disk is accessed by physical
  sectors rather than by logical block. In this case, `LBLOCK `contains
  the sector number, and `NBYTES `must be zero. The whole sector will be
  transfered, whatever its size.
  A = ASSYNC. If this bit is 1, access will be assynchroneous. This
  should never be the case, so leave this bit as 0.
In summary, the stack would look like this before the call:

                            <
             | Ctrl word   |
             | Log Block   |
             | Num bytes   |
             | Byte offset |
             | Word base   |
             | Unit number |
             |/////////////|

After the call, all parameters will be poped from the stack by UNITREAD
itself.

####  Unitwrite

This procedure writes a given number of bytes to a device. Its format is
exactly identical to that of UNITREAD.

####   Unitclear

This procedure is used to reset a device. At the RSP level, this means
clearing any state flag. The device can then be reset to a known state
by the BIOS, but this is a device-dependent operation. The Pascal
declaration is:

    PROCEDURE UNITCLEAR(UNITNUMBER: INTEGER);

####  Unitstatus

This procedure is intended for a device to report its status into a
buffer that can be upto 30 words in length. The meaning of these words
will depend on the device. The Pascal declaration of `UNITSTATUS `is:

    PROCEDURE UNITSTATUS(UNITNUMBER: INTEGER;
                         VAR STATUS_WORDS: ARRAY[0..29] OF INTEGERS;
                         CTRL: INTEGER);

**`UNITNUMBER`** is the unit number as defined above.
**`STATUS_WORDS`** is a pointer to a memory buffer that can accomodate
upto 30 integers.
**`CTRL`** is control word. Only 1 bit is defined:

    UUUr rrrr rrrr rrrD

- U = user defined control-bits.
  r = reserved bits.
  D = direction. If the bit is 1, the status of the input channel is
  queried. If this bit is 0, the status of the ouput channel is queried.

####  Unitbusy

This procedure was intended for assynchronous environments and will
therefore always return "false" with the TI-99/4A system. Its Pascal
declaration is:

    FUNCTION UNITBUSY(UNITNUMBER: INTEGER): BOOLEAN;

####  Unitwait

This procedure is intended for assynchronous environments. On the
TI-99/4A system, it returns immediately and can thus be considered as a
NOP. Its Pascal format is:

    PROCEDURE UNITWAIT(UNITNUMBER: INTEGER);

###  Special characters

As already stated, the main responsability of the RSP/IO is to handle
communications with the BIOS in a standardized manner. But in addition,
the RSP/IO is also in charge of processing a few special characters: two
during output (`CR/LF` and `DLE`), and two during input (`EOF `and
`ALPHALOCK`).

#### CR / LF

With the p-system, lines in a text file end with a single `CR `character
(\>0D), but this has the meaning of "carriage return, plus line feed".
Many devices consider these two functions as distinct: `CR `brings the
cursor left, while `LF `(ascii \>0A) moves one line down. The RSP/IO
will thus append a `LF `to every `CR `while outputing a text file. This
behaviour can be suppressed by setting the NOCRLF bit as 1 in the
control word.

#### DLE blank compression

When a text file contains a long stretch of spaces,such as an indented
line, they are compressed in two bytes: the first one is the
`DLE `character (ascii \>10), the second is the number of spaces plus 32
(so as to make it a printable character). For instance, a line indented
by 8 would start with \>10 \>28. The RSP/IO must decompress such lines
and send the appropriate number of spaces to the BIOS, instead of the
`DLE `code.

#### EOF

Several devices, such as CONSOLE, PRINTER or REMIN send a special
character, called `EOF`, to signal that the end-of-file has been
reached. In the p-system, `EOF `doesn't have a fixed ascii value.
Rather, it is a variable stored in the SYSCOM area (a portion of memory
shared by the system and the PME). The RSP/IO must therefore compare
each incoming character to the one in SYSCOM and, if they match, process
it as an `EOF`. With the TI-99/4A, the value of EOF is generally \>83,
which corresponds to Ctrl-C.

The meaning of an `EOF `depends on the device: for CONSOLE the rest of
the input buffer will be filled with zeros, for the printer and remote
devices, the `EOF `character is copied into the buffer. In any case,
input is terminated immediately.

#### ALPHALOCK

Apart for the hardware "alpha-lock" key on the TI-99/4A keyboard, the
p-system also provides a software-based alpha-lock. When the special
`ALPHALOCK `character is received, all following lower-case characters
('a' through 'z') will be converted to upper-case. This will continue
until another `ALPHALOCK `character is received. As with `EOF`, there is
no fixed ascii value for this character, rather it is defined by a
variable in the SYSCOM area (with the TI-99/4A, the ascii value of this
variable is generally \>0C, i.e. FCTN-6).

As mentionned above, it is possible to disable the handling of `EOF `and
`ALPHALOCK `by setting the NOSPEC bit to 1 in the control word. Note
however that this will also disable the handling by the BIOS of the
other special characters (break, start/stop, flush and character
masking).

###  Subsidiary volumes

The p-system offers the possibility of creating virtual drives, that are
physically implemented as a partition of an actual drive. These are
called subsidiary volumes. Another responsability of the RSP/IO is to
intercept calls to a subsidiary volume and to translate them into the
appropriate call to the physical unit on which the subsidiary volume is
implemented. Concretely, this means changing the unit number and
adjusting the block address.

The SYSCOM memory area contains a unit table with an entry for each unit
in the system. For storage devices, this record contains a physical disk
unit and a block offset. The RSP/IO looks-up in this table for the
proper unit number to use and adds to block offset to the LBLOCK
parameter passed by the caller. After some sanity check for absent
devices or illegal block number, it calls the BIOS with the new values.

------------------------------------------------------------------------

The BIOS

The BIOS (Basic Input Output Subsystem) is of course machine-specific.
It is in charge of interfacing the computer hardware with the PME via
the RSP/IO and with p-code programs via the RSP/IO or directly. It
retains the basic definitions of units, but does not have to handle them
in a standardized manner.

In general, each device driver in the BIOS will handle four operations:
read, write, control and status. However, the parameters required for
the corresponding function may vary according to the device.

All BIOS functions should return a completion code to the RSP/IO, so
that it knows if anything went wrong. These codes are a subset of those
that can be returned by the IORESULT routine of the RSP/IO. The
following codes can be returned by the BIOS:

0: No error
1: CRC error
2: Bad device number (e.g. unimplemented use-defined device)
3: Illegal I/O request
4: Undefined hardware error (not the same as IORESULT)
9: Device not on line (or not implemented in BIOS)
15: Ring buffer overflow
16: Disk is write-protected
17: Illegal block number
18: Illegal buffer address
128-255: Hardware-specific error.

[Console
](#Console%20BIOS)[Printer
](#Printer%20BIOS)[Disk drives
](#Disk%20BIOS)[Remote
](#Remote%20BIOS)[Serial
](#Serial%20BIOS)[User-defined
](#User%20BIOS)[System](#System%20BIOS)

### Console BIOS

On the TI-99/4A, the default CONSOLE input channel is the keyboard,
while the default output channel is the screen. Note however that it is
possible to redirect each channel to another device (e.g. output to a
printer or input from a file).

#### CONSOLEREAD

Only one parameter is needed for reading from the CONSOLE:

The byte to be transfered. This character is not echoed to the screen by
the BIOS, but by the RSP/IO.

CONSOLE is expected to contain a type-ahead buffer, able to store from 1
to 32 characters (or more) received from the keyboard until they are
passed to the RSP. If additional characters are entered from the
keyboard once the buffer is full, the bell should ring.

Optionally, all characters input from the console can be masked with
\>7F, as the left-most bit is always 0 in ascci. However, to accomodate
special characters, it is possible to mask the input with \>FF, i.e. to
leave the characters unchanged. The value of the mask is set by the
SETUP utility and stored in the SYSCOM area, an area of memory shared by
the PME and the p-system.

In addition, the BIOS recognises the following control characters that
regulates the output channel:

`STOP`: suspends ouput to CONSOLE, until another `STOP`, a `BREAK `or a
`FLUSH `character is received, or the console is reinitialized. If
another `STOP `is received, ouput should resume normally from where it
stopped. This allows to freeze the screen when data scroll up too fast
to be read.

`FLUSH`: suspends output to CONSOLE and discards all characters until
another flush or a break character is received, an input from CONSOLE is
requested, or the console is reinitialized. This allows to quickly
terminate lengthy output. Note that `FLUSH `has priority over
`STOP `(i.e. has the same effect whether output is frozen or not).

`BREAK`: if the NOBREAK flag in the SYSCOM area (value \>40) is set to
1, nothing happens. Otherwise, this character causes execution of a
"break" routine whose address is determined at startup time. Once this
routine returns, the BIOS continues as before. Note that the
`BREAK `character is not passed to the RSP: it is the responsability of
the "break" routine to inform the RSP about the break.

#### CONSOLEWRITE

Only one parameter is needed for reading from the CONSOLE:

The byte to be displayed.

But the BIOS must also process some characters that have a special
meaning:

`CR `(ascii \>0D): brings the cursor to the beginning of the current
line, in column 0.

`LF `(ascii \>0A): brings the cursor down one line, in the same column.
If necessary, scrolls the screen up.

`BEL `(ascii \>07): generates a beep (if possible).

`NUL `(ascii \>00): causes a delay equal to the time needed to output a
character, but does not output anything.

Optionally, some additional functions can be offered by the BIOS. There
is no fixed ascii code for these control characters, their value is
determined at startup time by entries in the SYSTEM.MISCINFO file.

`Move-up`: moves the cursor one line up, in the same column. If
necessary, scrolls the screen down.

`Move-left, move-right`: two control characters that move the cursor
forward or backward respectively, without modifying the text. The
behaviour at the beginning or the end of the line depends on the device
(normally, the cursor stays where it is).

`Home`: moves the cursor to the upper left-hand corner of the screen,
with no changes to the text.

`Move-to`: moves the cursor to an X,Y position without altering the
text.

`Erase to EOL`: erases the current line from the cursor position to the
end of the line. Leaves the cursor where it is.

`Erase to end-of-screen`: erases from the cursor position to the end of
the screen. Leaves the cursor where it is.

The effect of other non-printable characters is not strictly defined in
the p-system specifications.

#### CONSOLECTRL

The CONSOLE initialisation function should be passed two parameters:

The "break" vector

The SYSCOM pointer

The p-system stores the values for the special characters, the character
mask, and some flags into an area of memory shared with the PME, called
the SYSCOM area (this area is part of the Kernel segment). Upon
initialisation, a pointer to SYSCOM is passed to the console BIOS. From
this pointer, the offsets of the various control characters are the
following:

\>003A: `NOBREAK` (bit value \>40)
\>0052: `EOF `character
\>0053: `FLUSH `character
\>0054: `BREAK `character
\>0055: `STOP `character
\>005C: Character mask (\>7F or \>FF)
\>005D: `ALPHALOCK `character

Another parameter passed to the console BIOS upon initialisation is the
address of the "break" routine, to be called upon reception of a
`BREAK `character. The console BIOS should store this address for
further calls.

Upon initialisation, the console BIOS should reset any pending flushed
or stopped output status and clear the type ahead buffer.

#### CONSOLESTAT

The status routine requires two parameter:

The address of a memory buffer where to report the status

The control word that determines whether the input (1) or output (0)
channel status is wanted

Only the first status word in the buffer is used:

- \>0000: Number of characters currently in the input or output buffer
  (depending on the channel tested). If there is no output buffer, this
  word always contain zero. If there is no input buffer this word will
  be 1 if a character is ready to be read, 0 otherwise.

###  Printer BIOS

#### PRINTERWRITE

Only one parameter:

Byte to print

So as to be as general as possible, the RSP sends characters to the
printer one at a time. If the printer expects characters to be passed as
whole lines, it is the BIOS job to buffer the characters until an EOL
(end-of-line) is encountered. Valid EOL characters are:

`CR `(ascii \>0D): returns the carriage to column 1 but does not advance
the paper

`LF `(ascii \>0A): causes the paper to advance to the next line. The RSP
always sends a `LF `after a `CR`. If the printer cannot handle them
separately, the BIOS should ignore the `CR `and perform a "new line"
when it receives the `LF`.

`FF `(ascii \>0C): causes the paper to advance to the top of the next
page and performs a `CR`. If the printer does not offer this
possibility, the BIOS only performs a new line (i.e `CR `+ `LF`).

####  PRINTERREAD

Only one parameter:

Byte received

If the printer is able to return data, the BIOS will pass them directly
to the RSP. Otherwise, input operations result in an error code 3
("illegal operation").

#### PRINTERCTRL

Upon initialisation, the character buffer is emptied and a new line
(`CR `+ `LF`) is performed. No parameter is passed.

#### PRINTERSTAT

As is the case for the console, the printer status function takes two
parameters:

A pointer to a memory buffer where status words have to be placed

A control word that determines which channel is to be checked (1=input
or 0=output)

If the printer has no self-checking abilities, this function just
returns 0. For the output channel, the BIOS returns the number of
characters remaining to be printed, zero is interpreted by the RSP/IO as
a signal that the printer is ready to print. A non-zero value indicates
that sending a character to the printer may hang the system until the
printer is ready.

###  Disk BIOS

Disks devices are considered by the RSP as a linear array of 512-byte
records. The BIOS is in charge of converting this abstraction to actual
parameters needed for disk access. Note that this feature can be
overriden by setting the PHYSSECT bit in the control word so that the
RSP itself can do hardware-level operations (i.e. pass the actual sector
number).

#### DISKREAD

This function takes five parameters:

The logical block number

The number of bytes to transfer (0 to 32767)

The address of the data buffer

The drive number (0 for DSK1, 1 for DSK2, etc).

The control word as defined in [`UNITREAD`](#unitread)` `in the RSP/IO
section.

On input operations, the BIOS will only transfer the required number of
characters, even if this is not a complete sector.

#### DISKWRITE

This function takes the same five parameters as DISKREAD.

For output operations, if the number of bytes sent is not a multiple of
512, the remaining bytes are undefined. This means that the end of the
"block" is likely to contain garbage. It is the responsability of the
application program, to know where the valid data ends in the block.

#### DISKCTRL

Only one parameter is passed:

Drive number

Initialisation brings the drive to a state where it is ready to read or
write. Any buffered data that has not been saved to disk yet will be
lost.

#### DISKSTAT

The status function takes three parameters:

The drive number

A pointer to a memory buffer where status must be reported

A control word the defines which channel is to be tested (1=input,
0=output)

For disks, the status report consists in 4 words:

- \>0000: the number of bytes currently in the input or output buffer
  (depending on the channel tested)
  \>0002: the number of bytes per sector.
  \>0004: the number of sectors per track.
  \>0006: the number of tracks per disk.

#### Disk format issues

The BIOS is in charge of converting the real, physical sectors, into
logical 512-bytes sectors, no matter what the actual sector size may be.
This may require some tricky buffering if the physical sector size is
greater than 512 bytes...

Generally, the BIOS will stay away from the first two physical sectors
on the disk, since these are meant for the boot program. On the
TI-99/4A, the BIOS fills the whole floppy with one huge file so that all
disk operations from the p-system translate into file operations for the
TI-99/4A DSRs. The first three sectors on the disk will be occupied by
the disk directory, the list of file and the file descriptor record
(FDR), which leaves 357 sectors of 256 bytes available for the p-system.

Provision was made for the p-system to access physical sectors directly,
with `DISKREAD `and `DISKWRITE`. To this end, you must set bit 1 (weight
\>02) in the control word. The number of bytes should be set as 0 since
the whole sector will be transfered, no mater what size it is. Finally,
the physical sector number must be passed as the first parameter,
instead of the logical sector number. (NB The p-system designers
reserved the right for the "number of bytes" parameter to be combined
with the sector number, so that one could access more than 65536
sectors. Which is why it should be 0 for now).

The correspondance between logical and physical sectors is for you to
calculate, using the following formula:

    PhysicalSector = (TrackNumber * SectorsPerTrack) + SectorNumber -1

    TrackNumber = PhysicalSector / SectorsPerTrack

    SectorNumber = ( PhysicalSector mod SectorsPerTrack) +1

Where "mod" is the modulo operation, i.e. the remainder of the rounded
division. Note the addition/substraction of 1, due to the fact that
physical sectors are numbered from one on, whereas logical sectors are
numbered from zero on. No correction is necessary for tracks, since they
are numbered from zero on.

###  Remote BIOS

This device is inteded to be a RS232 serial connection. The BIOS should
not alter any character during transmission (e.g. not mask the most
significant bit). The usual transfer rate is 9600 bauds.

#### REMOTEREAD

Single parameter passed:

Byte received

Since they are read one at a time, input characters are buffered as for
the keyboard, in a "type-ahead" buffer.

#### REMOTEWRITE

Only one paramter is needed:

Byte to send

Bytes are sent one at a time by the RSP. The BIOS transmits them as
such.

#### REMOTECTRL

Initialisation brings REMOTE to a state where it is ready to both send
or receive. No parameter is passed.

#### REMOTESTAT

The status function takes two parameters:

A pointer to a memory buffer where status must be reported.

A control word the defines which channel is to be tested (1=input,
0=output)

Only the first word of the status buffer is used:

- \>0000: number of characters currently stored in the input (or output)
  buffer.

###  Serial devices

For instance, the serial COM port on a PC.

#### SERREAD

Two parameters are passed:

The device number

The byte to be read

#### SERWRITE

Two parameters are passed

The device number

The byte to send

#### SERCTRL

One parameter is used:

Device number

#### SERSTAT

Three parameters are required:

The drive number

A control word the defines which channel is to be tested (1=input,
0=output)

A pointer to a memory buffer where status must be reported

###  User-defined devices

The BIOS implementation of a user-defined device is left to the user.
The only requirments is that the function return a completion code when
finished, and that an error code 2 ("illegal unit number") is returned
if the UNITNUMBER parameter does not match any device. User-defined
devices should have numbers greater than 127.

#### USERREAD

Five parameters are passed to this function. The BIOS can choose to
ignore any of them, of course:

The logical block number

The number of byte to transfer (0 to 32767)

The address of the data buffer

The device number, which corresponds to `UNITNUMBER `in the RSP call.

The [control word](#unitread) as defined by the RSP/IO (in
`UNITREAD `and `UNITWRITE`).

#### USERWRITE

This function takes the same parameters as USERREAD.

#### USERINIT

Only one parameter is passed to this function:

Device number

What it does is up to the user.

#### USERSTAT

This function is passed three parameters:

The device number

A pointer to a memory buffer where status must be reported

A control word the defines which channel is to be tested (1=input,
0=output)

What it reports is up to the user that defined the device.

###  System BIOS

Although the operating system is not an I/O device, the RSP can still
use the standard I/O function to address the system BIOS. Each function
has a particular meaning:

#### SYSREAD

This function is reserved for future extension. Calling it causes an
HALT (i.e. exits the p-system). Five parameters are passed:

The logical block number

The number of byte to transfer (0 to 32767)

The address of the data buffer

The device number

The [control word](#unitread) for the RSP/IO.

#### SYSWRITE

Reserved for future extension. Causes an HALT (i.e. exits the p-system).
The same five parameter are used than with SYSREAD.

#### SYSCTRL

The function is passed two parameters:

The device number

The EVENT vector

Resets the clock value as \>00000000.

#### SYSSTAT

Two parameters are required:

A pointer to a memory buffer where status must be reported

The control word

Returns 3 words in the status buffer:

- \>0000: Address of the last word (not the last byte! This must be an
  even address) of RAM accessible for the system.
  \>0002: The least significant word of a 32-bit time value. If not
  clock is implemented, this word is 0. Otherwise, it contains the
  number of 1/60th of seconds since initialisation.
  \>0004: The most significant word of the time value or zero if the
  internal clock in not implemented.
------------------------------------------------------------------------

The Operating system

The operating system is in charge of loading and running p-code programs
in the host machine. One of its main tasks is to react to segment
faults: each time the p-code program calls a segment that is not
currently in memory, a segment fault is issued. The p-system kicks in,
loads the required segment in the code pool, (possibly shifting other
segment to make room, or even trashing one), then it returns control to
the p-code program.

The p-system manages a special memory stack, called the heap, for its
internal use. Under some circonstances, the user can also store data on
the heap.

[System units
](#system%20units)[Code Pool
](#Code%20pool)[Faults
](#Faults)[Heap
](#Heap)[SIBs
](#SIB)[E-Recs and E-Vects
](#E_Rec)[Tasks
](#Tasks)[Syscom area
](#Syscom%20area)[File access](#File%20access)

###  System compilation units

The p-system is written in Pascal. It consists in the following
compilation units:

Units
Usage

HEAPOPS
EXTRAHEAP
PERMHEAP
Heap operations
SCREENOPS
Screen operations
FILEOPS
File and directories operations
PASCALIO
EXTRAIO
SOFTOPS
File-level I/O
SMALLCOMMAND
COMMANDIO
I/O redirection and chaining
STRINGOPS
String operations
OSUTILS
Conversion utilities
CONCURRENCY
Concurrency management
REALOPS
Floating point operations, real numbers
I/O
LONGOPS
Long integer operations
GOTOXY
Screen cursor control (may be
user-supplied)
KERNEL
Resident part of the OS (segment 0). Code
pool maintenance, fault handling, segments loading.
GETCMD
Subsidiary segment of KERNEL ( swappable).
Processes user input, builds run-time environment.
USERPROG
Ditto. Contains user program (at power-up:
builds the initial environment). Segment 15.
INITIALIZE
Ditto. Processes SYSTEM.MISCINFO, locates
code files, builds the device table.
PRINTERROR
Ditto. Prints run-time error
messages.

###  Code pool

The code pool is where p-code segments are placed for execution. It is
managed by the system which can move or unload segments if more room is
needed. The system always keeps the loaded segment as a continous block
in memory. When new segments are loaded, they are appended at the end of
this area. Note that assembly language segments, which are not
relocatable, are not placed in the code pool , but rather on the heap.

Depending on the machine, the code pool can be placed in between the
stack and the heap (internal pool), or have a memory area for itself
(external pool). I'm not sure which is the case for the TI-99/4A.

The following variables are used by the code-pool management routines:

**SP_Low**: Only relevant for an internal pool. Pointer to the lowest
possible bound of the stack, i.e. one word above the top of an internal
code pool. SP_Low is located in the TIB (Task Information Block).

**HeapTop**. Only relevant for an internal pool. Pointer to the top of
the heap. HeapTop is part of the HeapInfo record.

**Seg_Pool**. Pointer to the Pool_Descriptor. Seg_Pool is part of the
[SIB](#SIB) (Segment Information Block).

The Pool_Descriptor comprises the following fields:

    Record
      PoolBase   : fulladdress;
      PoolSize   : integer;
      MinOffset  : memptr;
      MaxOffset  : memptr;
      Resolution : integer;
      PoolHead   : SIB_Ptr;
      Perm_SIB   : SIB_Ptr;
      Extended   : boolean;
    end;

**PoolBase** is a 32-bits address that points at the base of the code
pool.
**PoolSize** is the size in words of the code pool. Set by the SETUP
utility.
**MinOffset** is the lower boundary of the code pool.
**MaxOffset** is the upper boundary of the code pool. Same as SP_Low for
an internal pool.
**Resolution** is the offset in bytes for segment alignment, set in
SYSTEM.MISCINFO. Segments always start at an address which is a multiple
of this number.
**PoolHead** points at the SIB at the base of the code pool.
**Perm_SIB** points at a SIB that is always resident (currently
GOTOXY).
**Extended** is TRUE if extended memory is used. Set in SYSTEM.MISCINFO.

When a segment fault occurs, the pool managment routines must load the
required segment(s) before returning to the PME. The routines
successively try to:.

Load the segment at either end of the code pool.

If the pool is internal, shift it towards the stack (or the heap), to
try to make room at the other end. The pool can be moved all the way to
the heap, but never closer than 40 words from the top of the stack.

Swap out swappable segments and group the others together, to make room
for the incoming segment(s).

If even this fails, report a stack overflow, and reinitialize the
system.

With an internal code pool, the managment routines must also respond to
stack fault and heap faults. To free the necessary space, the routines
try to:

Move the pool toward the stack, to free the required number of words for
the heap (or conversely).

Swap out one or more swappable segment(s), compact the others together
and shift the pool towards the stack (or heap).

If this does not work, report a stack overflow and reinitialized the
system.

###  Fault handling

There is a high priority task called FaultHandler in the OS, whose job
is to handle memory fault. This process is started at boostrap time, but
immediately waits for a semaphore. When a routine detects a fault it
just signals the semaphore and FaultHandler takes over. FaultHandler
deals with the fault (hopefully), then resumes waiting for the
semaphore, which returns control to the faulty task.

The semaphore resides in the SYSCOM area and is declared as:

    Fault_Message = RECORD
                     Fault_TIB  : TIB_Ptr;
                     Fault_E_Rec: E_Rec_Ptr;
                     Fault_Words: integer;
                     Fault_Type : SegFault..Pool_Fault;
                    END;

    Fault_Sem: RECORD
                Real_Sem, Message_Sem: semaphore;
                Message: Fault_Message;
               END;

As you may have guessed, the record "Message" is used by the routine
that generated the fault to pass information to FaultHandler. The
contents of this record is pretty much self-explanatory, but just in
case:

**FaultType** indicates the type of fault (segment, stack, heap, pool,
etc).
**TIB_Ptr** points to the Task Information Block of the faulting task.
**Fault_E_Rec** points to the Environment record of the current segment
or of the missing segment (for segment faults).
**Fault_Words** is the number of words needed (e.g. for stack faults).
It's 0 for segment faults.

Faults can be signaled either by the PME or by the OS itself. The PME
detects only stack and segment faults, which have been described in more
details in the relevant [chapter](#Faults&errors).

The OS routes all faults through the EXECERROR routine. It can issue a
segment fault in only one occasion: when you try to use MEMLOCK to lock
a segment that's not in memory. It can also issue heap faults, when you
are using MARK, NEW, VARNEW or PERMNEW and there isn't enough memory on
the heap.

As discussed above, FaultHandler then tries to move around segments in
the code pool, and free some memory. When doing this, it must of course
make sure that the current segment will not be swapped out of memory! In
case of a stack fault created by calling a routine in a different
segment, both segments must be locked in memory.

###  The Heap

The heap is a separate stack used by the OS for its own purposes.
Typically, the heap and the stack grow toward each other in memory,
possibly with the code pool in between them. Which means that stack
faults, and possibly segment faults, can affect the amount of memory
available for the heap. Conversely, the heap management routines can
issue a heap fault to request additional space.

The heap managment routines are the following:

#### MARK();

Saves the location of the top-of-heap.

#### RELEASE()

Cuts the heap at the position of the corresponding MARK, discarding any
variable located in this space (except those declared with PERMNEW).
MARK and RELEASE can be nested, and care should be taken to match them
properly.

#### NEW(PTR: TYPE)

Allocates room for a variable on top of the heap. PTR is a pointer to a
variable type, which allows NEW to know how many words should be
reserved. If TYPE has variants, NEW allocates enough space for the
largest variant, unless instructed otherwise.

#### DISPOSE(PTR: TYPE)

De-allocates the space reserved by NEW for the variable pointed by PTR.
Afterwards, PTR will contain NIL. If a specific variant of TYPE was
requested with NEW, then it should also be requested in the
corresponding DISPOSE.

#### VARNEW(PTR: TYPE; NWORDS: word): word

Allocates NWORDS words on top of the heap and places the address of the
first one in the variable pointed by P. Returns the number of words
effectively allocated which should be equal to NWORDS, or to zero if
there wasn't enough space.

#### VARDISPOSE(PTR: TYPE; NWORDS: word)

De-allocates NWORDS words from the position pointed at by PTR and places
NIL in PTR. It's extremely important that NWORDS has the exact same
value than was unsed in the corresponding VARNEW call.

#### PERMNEW(PTR: TYPE)

Allocates memory for a variable just like NEW, but this space can only
be reclaimed with PERMDISPOSE: even RELEASE won't destroy it. This
function is reserved for internal use by the OS (e.g to chain programs,
so that the next command is still available after a program has released
its heap allocation).

#### PERMDISPOSE(PTR:TYPE)

This is the only way to de-allocate space reserved by PERNEW. Again,
this procedure is reserved for system use.

These procedures are located in three compilation units:

HEAPOPS contains MARK, RELEASE and NEW.
EXTRAHEAP contains DISPOSE, VARNEW , VARAVAIL, MEMLOCK and MEMSWAP.
PERMHEAP contains PERMNEW, PERMDISPOSE and PERMRELEASE.

###  Heap management

The heap is managed by placing marks on it. There are two types of
marks:

Marks placed by the MARK routine and used by RELEASE to cut the heap

Marks placed by NEW and VARNEW to reserve heap space for data.

Both types of mark share the common type MemLink:

    TYPE
     MemLink = RECORD
                Avail_list: MemPtr;
                NWords: integer;
                CASE Boolean OF
                  true: (Last_Avail, Prev_Mark : MemPtr);
               END

With MARKed marks, the variant is true, and we have the following
fields:

**NWords** is always 0 because these marks do not reserve any space.
**Prev_Mark** points to the previous mark down the heap.The bottom mark
has NIL in this field and the topmost mark is pointed at by the global
variable HeapInfo.TopMark. This allows to easily walk the whole chain of
marks on the heap, for instance to remove a mark with RELEASE.
**Last_Avail** points to the top of the available space above the mark.
Typically, this space will be bound by the next mark, or by an allocated
variable (or by the code pool, in the case of the topmost mark on the
heap).
**Avail_list** points to a list of data marks, created by either NEW or
VARNEW. The first record in this list constitues the lowest unallocated
space above the mark.
With data marks, the variant field is false, so Last_Avail and Prev_Mark
don't exist:

**NWords** is the amount of space reserved for the variable, including
the two words occupied by the mark record itself. **Avail_list** points
to the memory-reserving mark for the next variable.

####  Global variables

**HeapInfo** is an important variable used for heap maintenance. It has
the following structure:

    VAR
     HeapInfo: RECORD
                Lock: semaphore;
                Topmark, HeapTop: MemPtr;
               END

     PermList: MemPtr;

**Lock** is a semaphore used by heap-modifying routines, to make sure
that the heap will only be modified by one process at a time.
**TopMark** point to the topmost mark installed by the MARK subroutine,
as described above.
**HeapTop** points at the highest allocated space on the heap. It is
used by FaultHandler to determine how close the heap is from the stack
(or from the code pool, if it's placed in between the heap and the
stack).

**PermList** is another variable used for heap management. It points at
the top of a separate list of memory-reserving marks used uniquely for
variables allocated by PERMNEW. PermList can be NIL if no variable was
allocated with PERMNEW.

Example:

Here is a example of heap structure. It contains three MARKed marks
(red) linked together through their Prev_Mark fields (red arrows). In
each, the Last_Avail field points to the highest portion of the heap
that they are allowed to use for their data marks (black arrows).

The top mark's chain of data mark consist in only one mark, that
reserves space for the integer Dummy.

The bottom mark does not point to any data chain any more, but there is
some free space above it, indicating that some data may have been there,
but disposed of.

The middle mark point to a chain of data marks (blue) linked together by
their Avail_List fields (blue arrows). In this case, the chain contains
only two data marks, that reserve memory for the integer Foo, and the
3-word long-integer Bar, respectively. Note that here also, there is
some empty space between Foo and Bar, where variables may have been
allocated, then disposed of.

    :                  :
    | STACK or CODE    |
    +------------------+ <---,
    | //////////////// |     |
    | //////////////// |     |
    +------------------+     |  <---HeapInfo.HeapTop
    | Int: Dummy       |     |
    +------------------+     |
    | Avail_List = NIL |     |
    | NWords = 1       |     |
    +------------------+ <-, |
    | Avail_List  -----|---' |
    | NWords = 0       |     |
    | Last_Avail  -----|-----'
    | Prev_Mark   -----|-------,
    +------------------+       |<---HeapInfo.TopMark
    | Long: Bar        |       |
    | ( 3 words )      |       |
    +------------------+       |
    | Avail_List = NIL |       |
    | NWords = 3       |       |
    +------------------+<-,<-, |
    | //////////////// |  |  | |
    +------------------+  |  | |
    | Int: Foo         |  |  | |
    +------------------+  |  | |
    | Avail_List  -----|--'  | |
    | NWords = 1       |     | |
    +------------------+<-,  | |
    | Avail_List  -----|--'  | |
    | NWords = 0       |     | |
    | Last_Avail  -----|-----' |
    | Prev_Mark   -----|----,  |
    +------------------+<-, |<-'
    | //////////////// |  | |
    +------------------+  | |
    | Avail_List = NIL |  | |
    | NWords = 0       |  | |
    | Last_Avail  -----|--' |
    | Prev_Mark = NIL  |    |
    +------------------+<---'

####  Heap managment tactics

HeapTop is set to point at the new top of heap, as soon as space on top
of the heap is requested by one of the heap management routines. This
way, if a stack fault occurs, there won't be any conflict between these
routines and FaultHandler.

The OS uses the heap for its own purposes, but the heap is also
available for your programs. To avoid conflicts, the system puts a mark
on the heap ( called EMPTYHEAP) after your program's runtime environment
has been built, but before the program is actually started. Once the
program terminates, the system releases EMPTYHEAP, therefore destroying
anything you may have placed on the heap (unless you used PERMNEWs,
which you shouldn't). Note that your program's SIBs, E_Recs and E_Vects
are part of the run-time environment and will therefore appear before
the EMPTYHEAP mark. So do global data in your program, and in any units
it uses.

Among other things, the OS maintains a disk directory on the heap. It is
pointed at by the global variable SysCom^.GDirP, but it is meant to be
invisible for you. So before any heap operation (except DISPOSE), the
system will remove this directory with DISPOSE and make its space
available again.

###  SIB: Segment Information Blocks

A segment is considered active if it may be used by the current program,
whether it is currently loaded in memory or still on disk. Each active
segment is described in a Segment Information Block (SIB) on the heap.
When a program requires a segment that is not yet active, an SIB is
created for it on the heap. All SIBs are arranged in a double-linked
list and reference each others via their Prev_SIB and Next_SIB fields.

The structure of a SIB is the following:

    RECORD
     Seg_Pool   : ^Pool_Des;
     Seg_Base   : memptr;
     Seg_Refs   : integer;
     Time_Stamp : integer;
     Link_Count : integer;
     Residency  : -1...maxint;
     Seg_Name   : PACKED ARRAY [0..7] OF CHAR;
     Seg_Leng   : integer;
     Seg_Addr   : integer;
     Vol_Info   : VIP;
     Data_Size  : integer;
     Res_SIB    : RECORD
                   Next_SIB : SIB_Ptr;
                   Prev_SIB : SIB_Ptr;
                   CASE boolean OF
                     TRUE: Next_Sort: SIB_Ptr;
                     FALSE: New_Loc: memptr;
                  END
     M_Type    : integer;
    END

**Seg_Pool** points to the Pool_Descriptor for the pool in which the
segment resides. If the segment is on the heap (as is the case for
non-relocatable segments) or if there is no external code pool, Seg_Pool
is NIL.
**Seg_Base** contains the byte offset of the segment within the code
pool, or the address of the segment on the heap. If the segment is not
in memory, Seg_Base is NIL.
**Seg_Refs** contains the number of outstanding calls to the segment. It
is incremented each time a routine in the segment is called by an
external segment, it is decremented when this routine returns to its
external caller.
**Time_Stamp** is used by the system to determined which segment is the
least recently used, when segments should be swapped out to make room in
the code pool. The [Syscom area](#Syscom%20area) contains a 16-bit
variable that is incremented every time a segment is exited. This
variable is copied in the Time_Stamp field of the segment exited.
**Link_Count** indicates how many external variables reference the SIB.
The operating system uses several such variables, including the Prev_SIB
and Next_SIB fields of other SIBs. Link_Count is incremented each time a
pointer is set to the SIB and decremented when this pointer is
destroyed. When Link_Count becomes zero, the SIB is removed from the
heap.
**Residency** is 0 for segments that are swappable. Values greater than
zero indicate that the segment is locked in memory. Residency is
incremented each time a program locks the segment (with the MEMLOCK
function), and decremented each time it is unlocked (with the MEMSWAP
function). A value of -1 indicates a segment that was position-locked at
loading time and cannot be made swappable.
**Seg_Name** contains the first 8 characters of the segment name.
**Seg_Len** is the number of code words in the segment, including the
relocation lists but not the reference lists.
**Seg_Addr** contains the number of the segment's first block on disk.
**Vol_Info** points to a volume information record that contains the
drive number and diskname for the disk where the segment resides.
**Data_Size** is the number of data words in the code segment's data
segment. This only applies to principal segments (for others Data_Size
is 0).
**Res_SIB** is used to maintain the code pool by waking the list of
SIBs. Its Prev_SIB field points to the previous SIB in the list,
Next_SIB points to the next SIB in the list. An extra field is used by
the code pool management routine to store temporary values: either
Sort_SIB or New_Loc.

###  E_Rec and E_Vect

Many p-code instructions require a segment number as an argument. For
instance, `CXL 12,3` calls procedure number 3 in segment 12. However,
there is no such thing as a library of all existing segments and their
corresponding numbers. In fact, only one segment has a fixed number:
KERNEL, which is segment number 0. For all others, the compiler assigns
each segment an arbitrary number, from 2 to 255 (number 1 is reserved
for the current segment). This may be a problem when trying to link
units that were compiled independently: the same segment may have been
given completely different numbers during the various compilations.

Rather than having the linker patch the code to match the segment
numbers, the p-system designers went for a more flexible solution.
Firstly, the compiler appends a segment reference list to the codefile
it creates. This is a list of segment names, together with the numbers
that the compiler has assigned to them. Then, when the file is loaded,
the system creates an environment vector (or E_Vect) for each segment.
An E_Vect is just an array of pointers to other segments. This way, the
segment numbers used by the p-codes are just indexes into the E_Vect. To
find the relevant segment, the PME just reads the pointer found at a
given position in the E_Vect.

The structure of an E_Vect is the following:

    RECORD
     Vec_Length : integer;
     Map        : array [1..Vec_Length] of ^E_Rec;
    END

**Vec_Length** indicates the number of entries in the E_Vect, i.e. the
number of segments it references (including itself).
**Map** is an array of pointers to other segments, arranged according to
the segment numbers assigned at compilation time.

If you thought that the pointers in the E_Vect would point at other
segments' SIBs, you were wrong. They point at special stuctures, called
"Environment Records" or E_Rec. Each segment has an E_Rec, that contains
pointers to important structures in this segment. The structure of an
E_Rec is the following:

    RECORD
     Env_Data : memptr;
     Env_Vect : ^E_Vect;
     Env_SIB  : ^SIB;
     CASE boolean OF
      TRUE: ( Link_Count : integer;
              Next_Rec   : ^E_Rec;
             )
    END

**Env_Data** points at this segment's global data, allocated on the heap
when the program is executed.
**Env_Vect** points at this segment's E_Vect, an array of pointers to
other segments' E_Recs.
**Env_SIB** points at this segment's [SIB](#SIB), which is allocated on
the heap when the program is executed.
The next two fields are only present in the E_Rec of the principal
segment in a compilation unit:
**Link_Count** indicates the number of other units that are using this
principal segment.
**Next_Rec** is used to maintain a chain of all active compilation
units. It points to the next unit in the chain.

Example:

Let's use three segments, "Tic", "Tac" and "Toe". Tic is a principal
segment that references Tac and Toe. Toe is a principal segment in
another compilation unit, it also references Tac. Tac is a subsidiary
segment, it does not reference any other segment. Note that each segment
has an entry for itself in the E_Vect, but I omited these arrows, for
clarity.

         "Tic" E_Rec
       +------------+
       | Env_Data   |      E_Vect   1     2     3
       +------------+      +-----+-----+-----+-----+
       | Env_Vect   |----->|  3  | Tic | Toe | Tac |
       +------------+      +-----+-----+--|--+--|--+
       | Env_SIB    |                     |     |
       +------------+                     |     |
       | Link_Count |           ,---------|-----'
       +------------+           |         |
    ,--| Next_Rec   |           |         |
    |  +------------+           |         |
    |                           |         '-----,
    |  ,------------------------'               |
    |  |                                        |
    |  +----------------------------------,     |
    |  |                                  |     |
    |  V  "Tac" E_Rec                     |     |
    |  +------------+                     |     |
    |  | Env_Data   |      E_Vect   1     |     |
    |  +------------+      +-----+-----+  |     |
    |  | Env_Vect   |----->|  1  | Tac |  |     |
    |  +------------+      +-----+-----+  |     |
    |  | Env_SIB    |                     |     |
    |  +------------+                     |     |
    |                                     |     |
    |  ,----------------------------------|-----'
    |  |                                  |
    |  V  "Toe" E_Rec                     |
    '->+------------+                     |
       | Env_Data   |      E_Vect         |
       +------------+      +-----+-----+--|--+
       | Env_Vect   |----->|  2  | Toe | Tac |
       +------------+      +-----+-----+-----+
       | Env_SIB    |               1     2
       +------------+
       | Link_Count |
       +------------+
       | Next_Rec =0|
       +------------+

Note that Tac is referenced as segment number 2 in Toe , but as segment
number 3 in Tic. Therefore, the instruction
CXL Tac,5 (which calls procedure 5 in Tac ) will be compiled as \>93,
\>03, \>05 in Tic, but as \>93, \>02, \>05 in Toe.

When executing CXL Tac,5 in the segment Tic, the PME reads
\>93,**\>03**,\>05, gets the third enty in Tic's E_Vect and obtains a
pointer to Tac's E_Rec. Then it locates procedure number 5 in Tac and
branches to it.

Similarly, when encountering a CXL Tac,5 in segment Toe (which is coded
\>93,**\>02**,\>05), the PME gets entry number 2 in Toe's E_Vect, which
is again a pointer to Tac. You get the idea?

Finally, note the chaining between the principal segments: Tic's Nec_Rec
points to Toe (red arrow). The corresponding entry for Toe is NIL, since
Toe is at the end of the list in our example.

###  Tasks

The p-system is a multitasking operating system. That's right, it turns
your TI-99/4A into a multitasking machine!

A task can be seen as a "line of execution" of a program. At any time
there can only be one task running (since the TI-99/4A only has one
processor), but there can be many more tasks pending. The p-system
achieves multitasking by quickly switching from one task to the next,
providing the illusion that all tasks run at the same time.

The "main task" is the thread of execution that starts the p-system,
runs any utility or user program, then exits the p-system. Anywhere in
this thread, subsidiary tasks can be launched that will execute
"concurently" with the main task.

Three things are required for multitasking:

1.  The task(s) body, i.e. the routine(s) that must run in this task,
    and its associated data.
2.  A TIB, or Task Information Block, into which the p-system saves
    values critical for the task.
3.  A task queue used by the p-system to schedule the next task. There
    may be several queues: one for the tasks ready to run and one for
    each semaphore (whether there are tasks waiting for it or not).

#### TIB: Task Information Blocks

A TIB is a structure allocated on the heap into which the p-system saves
some critical values from the present task, before it switches to
another. For instance, it saves the current instruction pointer (IPC)
and most of the other p-machine registers. These values will be restored
when the system switches back to this task.

The structure of a TIB is the following:

    RECORD
     Regs: PACKED RECORD
            Wait_Q      : ^TIB;
            Prior       : byte;
            Flags       : byte;
            SP_Low      : memptr;
            SP_Upr      : memptr;
            SP          : memptr;
            MP          : ^MSCW;
            Reserved    : integer;
            IPC         : integer;
            Env         : ^E_Rec;
            Proc_Num    : byte;
            TI_BIOResult: byte;
            Hang_Ptr    : ^Sem;
            M_Depend    : integer;
          END
     Main_Task  : Boolean;
     Start_MSCW : ^MSCW;
    END

**IPC** is the p-code instruction pointer. It is a segment-relative byte
pointer.
**Proc_Num** is the number of the currently executing procedure in this
task.
**MP** points to the local activation record (MSCW) for the task, on the
stack.
**SP** is the p-machine stack pointer.
**SP_Low** is the lower bound of the stack for this task.
**SP_Upr** is the upper bound of the stack for this task. Note that each
subsidiary task has its own private stack, allocated on the heap when
the task is started. By default this stack is 200 words, but this can be
specified when starting the task. Only the main task uses the system
stack.
**Env** is a pointer to the task's E_Rec (from which you can find the
the task's SIB).
**TI_BIOResult** is used to save an I/O result that is local to the
task.
**M_Depend** contains machine-dependent data maintained by the PME. It
is initialized as 0.

**Prior** is the task priority. It's a number from 0 (lowest priority)
to 255 (highest priority).
**Wait_Q** is used when the task is in a waiting queue. It points to the
TIB of the next task in the list.
**Hang_Ptr** is used when a task is blocked by a semaphore. It points to
the semaphore (it's NIL if the task is not blocked).
**Flags** are reserved for future use.

**Main_Task** is TRUE for root ("parent") tasks, FALSE otherwise.
**Start_MSCW** points to the MSCW of the routine that started this task.

####  Queues and semaphores

At any time, there is only one task running. All other tasks ready to
run are placed in a waiting queue. When it's time to switch tasks, the
p-system fetches the appropriate task from the queue, switches to it,
and places the current task back into the queue.

If necessary, tasks can be synchronized by using semaphores. A semaphore
is a variable that can be "grabbed" by a task. If another task attempts
to grab the same semaphore it will hang, i.e. the task will be suspended
until the original task has "released" the semaphore. At which point the
hanging task will be able to grab the semaphore and resume execution.

Actually, a semaphore can be grabbed an arbitrary number of times before
it causes a task to hang. Similarly, a semaphore may be required to be
released several times before is actually frees the task(s) it has
hanged. In most cases however, a "grab count" of one (mutual exclusion
semaphore, or "mutex") will do the job.

Each semaphore has an associated queue of tasks waiting for it, similar
to the queue of tasks ready to run. Note that this queue may be empty if
no task is waiting for this semaphore. Hanging and releasing a task
simply means moving it from the semaphore queue to the "ready-to-run"
queue, or conversely.

To grab a semaphore, use the p-code [WAIT](#WAIT). It decrements the
semaphore "grab count" or hangs the current task if the count is already
zero. Which means that the p-system places the current task in the
semaphore queue and switches to another task from the "ready-to-run"
list.

To release a semaphore, use the p-code [SIGNAL](#SIGNAL). It fetches the
task at the head of the semaphore queue and places it in the
"ready-to-run" queue. A task switch will only occur if this task has a
higher priority than the current one. If the semaphore queue is empty,
the count is incremented and nothing else happens. The same thing
happens if the semaphore count is negative: this allows to implement
semaphores that must be signalled several times before hanged tasks can
be released.

You can also use the standard procedure [ATTACH](#ATTACH) to
automatically link a semaphore to a preset PME event. When this event
occurs, the semaphore will be signaled. The standard procedures
[QUIET](#QUIET) and [ENABLE](#ENABLE) allow to globally disable or
enable event-attached signalisation.

####  Concurrency unit

The p-system comprises a compilation unit called CONCURENCY, which
implements Pascal processes (a process is the Pascal word for a task).
The unit contains 3 procedures called START, STOP, and BLK_EXIT. The
Pascal compiler automatically calls START in the initialisation code of
a process, and STOP in its exit code. The compiler also calls BLK_EXIT
in the exit code of a main process.

These procedures make use of a global variable called Task_Info, which
has the following structure:

    RECORD
     Lock      : semaphore;
     Task_Done : semaphore;
     N_Tasks   : integer;
    END

**Lock** is a semaphore used to ensure that only one task at a time will
be able to modify Task_Info.
**Task_Done** is a semaphore used to wait for the termination of
subsidiary tasks.
**N_Tasks** is the number of subsiduary tasks that have been created by
START.

**START**

This procedure is called to create a new task. It reserves space on the
heap for the task's TIB, stack, and activation record, and then executes
a WAIT p-code. This normally causes the p-system to switch to the new
process (unless there is one with a higher priority pending). The new
process performs its initialisation (i.e. copying its parameters) then
executes a SIGNAL to return to START, which in turn returns to its
caller.

**STOP**

This procedure records the termination of a task. It decrements
Task_Info.N_Tasks and signals Task_Info.Task_Done. It then creates a
dummy semaphore and waits for it, which causes a permanent task switch
for the terminated process.

**BLK_EXIT**

This procedure is called by the main tasks exit code and waits for
termination of all subsidiary processes. It waits for
Task_Info.Task_Done until Task_Info.N_Tasks becomes zero. In other
words, it waits until each subsidiary task has called STOP.

###  Syscom area

The SYSCOM area is a memory location where the p-system stores
importants variables that can be shared by the PME, the RSP, the BIOS
and the OS, as well as by your program. From the OS point of view, the
SYSCOM area can be considered as a Pascal packed record and is declared
as such in the interface of the KERNEL unit.

Low-level assembly routines on the other hand, must know the offset of
each variable with respect the the SYSCOM^ pointer that is defined when
the system powers up. The problem here is that some of the variables are
bytes, so their location will vary according to the byte sex. For
instance, here are the offsets of the some keyboard special codes:

    Ctrl char  Little endian  Big endian
    EOF          >52            >53   }
    FLUSH        >53            >52   }
    BREAK        >54            >55 }
    STOP/START   >55            >54 }
    CHARMASK     >5C            >5D   }
    ALPHALOCK    >5D            >5C   }
    NOBREAK      >3A            >3B

The pairs defined with braces are bytes in the same word. When the byte
sex changes, the two bytes are flipped whithin the word.

###  File access

We saw that the RSP/IO consider every unit as an array of blocks, an
illusion implemented by the BIOS. The OS takes things a level higher and
introduces the concept of files. There can be two types of files:
standard files that follow the Pascal conventions and are accessed with
the GET and PUT functions. And intereractive files that accept
characters one at a time. From the OS point of view, both types of files
share a common structure called the File Information Block, or FIB.

#### Structure of a FIB

    FIB = RECORD
           FWindow: Window_P;
           FEOF, FEOLN: Boolean;
           FState: (FJanW, FNeedChar, FGotChar);
           FRecSize: integer;
           FLock: semaphore;
           CASE FIsOPen: Boolean OF
            true: (FIsBlkd: Boolean;
                   FUNIT: UNITNUM;
                   FVID: VID;
                   FRptCnt: integer;
                   FMaxBlk: integer;
                   FModified: Boolean;
                   FHeader: DirEntry;
                   CASE FSoftBuf: Boolean of
                    true: (FNxtByte: integer;
                           FMaxByte: integer;
                           FBufChngd: Boolean;
                           FBuffer: PACKED ARRAY[0..FBlkSize] OF CHAR
                           )
                   )

**FWindow** points to the current character in the file.
**FEOF** is the end-of-file flag.
**FEOLN** is the end-of-line flag.
**FState** is the file type: FJanW is the standard Pascal type (Jensen &
Wirth), FNeedChar is an interactive file waiting for a character,
FGotChar is an interactive file with a character.
**FRecSize** is the size of a record, in bytes. It is 0 for unentered
files, -1 for interactive and text files.
**FLock** is a semaphore used to ensure that only one process at a time
can access a file.
**FIsOpen** is true when the file is opened, which makes more fields
relevant:

- **FIsBlkd** is true if the file resides on a storage device.
  **FVID** is the number of the volume.
  **FRepCnt** is the number of time the window value is valid before
  another GET is needed.
  **FNxtBlk** is the next relative block to access.
  **FMaxBlk** is the maximum number of blocks that can be accessed.
  **FModified** is true if the file was modified and the last access
  date should be modified.
  **FHeader** is a copy of the file's directory entry.
  **FSoftBuf** is true if soft-buffered I/O is used, which is the case
  for all files on storage devices, except unentered files. If it is
  true, yet another set of fields become relevant:

  - **FNxtByte** is the next byte
    **FMaxByte** is the last byte. These two are used for buffer
    management.
    **FBufChanged** is true if the buffer was modified and needs to be
    saved.
    **FBuffer** is of course the soft buffer itself.

#### Disk directory

On a disk or another storage device is a directory, which is nothing
more than an array of 78 directory record. The first record (number 0)
defines the directory itself, the others entries define files.

The structure of the directory record is:

DFirstBlk
DLastBlk
-
DFKind
Lenght (7)
D
I
R
A
E
DeovBlk
DNumFiles
DLoadTime
(Year)
(Month)
DLastBoot
(Day)
The directory entries for files look like:

DFirstBlk
DLastBlk
StatusBit
DFKind
Length (15)
N
A
M
E
_
O
F
_
T
H
E
F
I
L
E
DLastByte
(Year)
(Month)
DAccess
(Day)

------------------------------------------------------------------------

File formats

This section describes the file formats used by the p-system.

[Text files](#Text%20file)

**Code files  **
[\_Segment dictionary
](#Seg%20dict)[\_Segment header
](#Seg%20header)[\_Procedure dictionary
](#Proc%20dict)[\_Constant pool
](#Cte%20pool)[\_Relocation list
](#Reloc%20list)[\_Segment reference list
](#Seg%20ref%20list)[\_Linker information](#Linker%20info)

[Assembly files](#Assembly%20files)

### Text files

A text file should only contain ASCII characters. It starts with a
2-block header that is used by the Screen Oriented Editor. If any other
program uses a text file, the OS ignores the header. When a new file is
created, its header blocks are filled with zeros.

Text files always have an even number of block (so the smallest text
file is 4 blocks long, including the header). Each pair of blocks makes
up a page. Text does not wrap from one page to the next, if necessary
the end of the page is filled with zeros.

Each page contains lines terminated by \ (ascii \>0D).
Optionally, if a line starts with a number of spaces, these can be
compressed to save disk space. In this case, the line will start with
the blank compression code (ascii \>10), followed by the number of
spaces plus 32.

If you want to access text files from Pascal programs, use READ, READLN,
WRITE and WRITELN, although GET and PUT also work and follow the
Jansen&Wirth rules for text files.

###  Code files

Code files contain programs, or independently compiled chunks of
programs called UNITs. These can contain both p-code and assembly
language. In addition, the code file contains a lot of information on
the code it carries, how to link the various units, etc.

All in all, the structure of a code file is quite complex. It consists
in a segment dictionary, and a series of [code
segments](#Code%20segments). To make things worse, the structure of a
code segment varies slightly between p-code and assembly segments.

#### Segment dictionary

The first block in a code file is the segment dictionary. It can
describe upto 16 of the segments that are part of the file. If there are
more than 16 segments, extra blocks can be added to the dictionary.
These extra blocks will be interspersed with the segments within the
code file. Each block contains a link to the following block, so that
the chain can be travelled.

A dictionary block basically consists in 6 arrays, which each upto 16
entries:

[Disk info](#Disk%20info)

[Segment names](#Segment%20names)

[Miscellaneous segment info](#Seg%20misc%20info)

[Pointers to interface text](#interface%20text)

[Segment information](#Segment%20info)

[Segment familly](#segment%20familly)

Followed by some [dictionary info](#Dictionary%20info).

Lets examine these arrays in detail:

##### Disk info

    Seg_Dict = RECORD
               Disk_Info: ARRAY[0..15] OF
                RECORD
                 Code_Addr: integer;
                 Code_Leng: integer;
                END

**Code_Address** is the block where the segments starts, relative to the
beginning of the code file. Thus, segments always start on a block
boundary.
**Code_Length** is the number of words in the segment, including the
relocation list, but not the reference list.
All unused entries in this array will be zeros.

#####  Segment names

              Seg_Name: ARRAY[0..15] OF
                         PACKED ARRAY[0..7] OF CHAR;

These are the segment names, upto seven characters in length (truncated
if necessary). Unused entries are filled with spaces.

#####  Misc segment info

              Seg_Misc: ARRAY[0..15] OF
               PACKED RECORD
                Seg_Type: Seg_Types;
                Filler: 0..31
                Has_Link_Info: Boolean;
                Relocatable: Boolean;
               END

     Seg_Types=(No_Seg, Prog_Seg, Unit_Seg, Proc_Seg, Seprt_Seg);

**Seg_type** is the segment type. It will be No_Seg if this directory
entry is empty (e.g.if there are less than 16 segments). Prog_Seg
indicates the outer segment of a program, Unit_Seg the outer segment of
a separately compiled unit. Proc_Seg segments contain procedures
belonging to a program or unit. Seprt_Seg segments contain assembly
language.
**Has_Link_Info** indicates that the segment needs to be linked. The
necessary linker information is included whithin the segment, starting
on the next block boundary after the reference list.
**Relocatable** indicates whether the segment is dynamically
relocatable.If it is, it can be loaded anywhere in the code pool, and
moved around if necessary. This is always the case for p-code segments.
By contrast, statically relocatable segments will be loaded on the OS
heap and locked there throughout their existence. Unless special care
has been taken in writing them, all assembly segments are statically
relocatable.
**Filler** just reserves the remaining 5 bits in the packed record for
future extension.

#####  Pointers to interface text

            Seg_Text: ARRAY[0..15] OF integers;

This array contains the number of the block where the segment interface
(in plain text) begins. The interface can be anywhere whithin the
segment, provided its starts on a block boundary. It's basically a chunk
of text file and thus follows the text format, except that the number of
blocks does not need to be even. Its size is specified in the
Seg_Familly array. Note that only unit segments have an interface
section, all other segments will have zero in this array.

##### Segment info

            Seg_Info: ARRAY[0..15] OF
             PACKED RECORD
              Seg_Num: 0..255;
              M_Type: M_Types;
              Filler: 0..1;
              Major_Version: Versions;
             END

     M_Types = (M_Pseudo, M_6809, M_PDP_11, M_8080, M_Z_80, M_GA_440, M_6502, M_6800
                M_9900, M_8086, M_Z8000, M_68000, M_HP87);

     Versions = (Unknown, I, II, II_1, III, IV, V, VI, VII);

**Seg_Num**, surprisingly enough, is the segment number, in the range 0
to 255.
**M_Type** define the language the segment contains. If it's p-code, the
type will be M_Pseudo, and the type of p-machine will be indicated in
Major_Version. If the segment contains assembly language, the processor
for which it was written is indicated here.
**Major_Version** indicates which version of the p-machine the p-code
was written for. So p-code is not that portable, after all...

#####  Segment family

            Seg_Famly: ARRAY[0..15] OF
             RECORD
              CASE Seg_Types OF
              Prog_Seg, Unit_Seg:
               (Data_Size: integer;
                Seg_Refs: integer;
                Max_Seg_Num: integer;
                Text_Size: integer);
               Proc_Seg, Seprt_Seg:
                (Prog_Name: PACKED ARRAY[0..7] OF CHARS);
              END

This array contains information that depends on the segment type. For
program and unit segments, we have:
**Data_Size** is the number of words in this segment's base data
segment.
**Ref_Size** is the number of words in the segment's reference list.
**Max_Seg_Num** is the highest segment number that was assigned within
this compilation unit (regardless of whether the referenced segments are
part of the code file or not).
**Text_Size**, as we just saw, is the number of words in the interface
section of a unit segment. For program segments this entry is zero.

Procedure segments and assembly language segment have only one field
here:
**Prog_Name** is the name of their parent unit (truncated to 8
characters if necessary). If it is unknown, which will generally be the
case for assembly segments, this entry is filled with spaces.

#####  Dictionary info

After the 6 arrays are a few variables that contain dictionary related
info.

            Next_Dict: integer;
            Filler: ARRAY[1..2] OF integer;
            Checksum: integer;
            Ped_Block;
            Ped_Block_Count;
            Part_Number: RECORD A, B: integer; END
            Copy_Note: string[77];
            Dict_Byte_Sex: integer;
        END {of Seg_Dict}

**Next_Dict** is the number of the next dictionary block whithin the
file, if any. If there are no more blocks, it is zero.
**Part_Number** is an arbitrary number for the program, as defined by
its programmer.
**Copy_Note** contains an optional copyright string.
**Dict_Byte_Sex** always contains one. According to the machine on which
the file was created, it will be coded either as \>0001 or as \>0100.
This allows the system to determine the byte sex of the dictionary.
**Checksum**, **Ped_Block** and **Ped_Block_Count** are used by an
utility called QUICKSTART, that does not come with the TI-99/4A p-code
system. For this reason, it is not described here.
**Filler** reserves the last words in the block for futur use.

And if you think all this is complicated, wait till we take a look at
the structure of a code segment...

####  Code segments

Code segments can contain upto 255 routines, numbered from 1 to 255.
Within the segment is a procedure dictionary which holds pointers to all
the procedures in the segment.

Constants embedded within p-code are actually placed in a separate area
in the segment, so that bytes can be flipped if the byte sex of the
target machine is different from that of the machine the file was
created on.

Segments also contain a relocation list containing the addresses of the
words that should be patched when the segment is moved from one area of
memory to another.

Segments are assigned a name (8 characters long) and a number at
compilation time. The name is used by the OS to stitch together the
various compilation units referenced by a program. The number is used to
reference the segment at run time, as described [above](#E_Rec).

Finally, a segment can also contain informations for linking it with
other segments.

By order of appearance, here are the sections that may be contained
within a code segment (many are optional):

[Segment header
](#Seg%20header)[Procedure dictionary
](#Proc%20dict)[Constant pool
](#Cte%20pool)[Relocation list
](#Reloc%20list)[Segment reference list
](#Seg%20ref%20list)[Linker information](#Linker%20info)

####  Segment header

Each segment starts with a mandatory header containing pointers to the
various tables, and some segment-related info:

    Proc_Dic: integer;
    Reloc_List: integer;
    Seg_Name: PACKED ARRAY[1..8] OF CHAR;
    Byte_Sex: integer;
    Cte_Pool: integer;
    Real_Size: integer;
    Part_Number: RECORD A, B: integer; END

**Proc_Dic** is a pointer to the procedure dictionary. It's an offset in
words from the start of the segment.
**Reloc_List** is a pointer to the relocation list. Zero if there is no
relocation list.
**Seg_Name** is the name of the segment.
**Byte_Sex** is always 1. If the machine that loads the segment has a
different byte sex than the one which wrote it, Byte_Sex will be read at
256 (\>0100). This lets the OS know that bytes should be flipped whithin
this segment. Of course, p-code is byte-sex independent, but constants
embedded within the code are not and their bytes may need to be flipped.
So are the various pointers in this header and in the procedure
dictionary.
**Cte_Pool** is a pointer to the constant pool. It can be zero if there
is no constant pool in this segment.
**Real_Size** is the size of floating point numbers used by this
segment.
**Part_Number** is a record of two integers reserved for future use
(e.g. to hold a version number).

####  Procedure dictionary

The procedure dictionary is nothing more than a list of pointers to each
and every procedure in the segment. The list grows downwards, from the
address pointed at by Proc_Dic. In other words, procedure numbers can be
considered as negative indices whithin the dictionary. The top word in
the dictionary (index 0, pointed at by Proc_Dic) is the number of
procedures in the dictionary. There may be empty entries in the
dictionary, whose content is 0. These correspond to procedures that were
declared as EXTERNAL or FORWARD routines.

    +------------+                   +-----------+
    | # of procs |                   | exit code |<-,
    +------------+ <-- Proc_Dic      |           |  |
    | Proc #1    |                   :           :  |
    +------------+                   |   code    |  |
    | Proc #2    |                   +-----------+  |
    +------------+                   | Data_Size |  |
    | Proc #3 ---|-----------------> +-----------+  |
    +------------+                   | Exit_IC --|--'
    | Proc #4    |                   +-----------+
    +------------+
    |    etc     |

The procedures pointers are word offset relative to the start of the
segment, just like Proc_Dic itself. Which means that they may need to be
flipped if byte sexes do not match.

Procedures are aligned on word boundaries and always begins with two
words.

    Exit_IC: integer;
    Data_Size: integer;

The pointer in the procedure dictionary points at Data_Size, which is
immediately followed by the executable code, whether p-code, assembly or
a mixture of both.

Data_Size is the number of words of local space that the procedure needs
for its variables. It does not include the calling parameters that are
assumed to be already on stack when the procedure is called. No local
space is reserved for pure assembly routines. For mixed language
routines, Data_Size is negative if the first instruction is in assembly.

Exit_IC points to the code that must be executed when a p-code procedure
is terminated. It is a offset in bytes (not words!) from the start of
the segment. Exit_IC is undefined for assembly language routines, and
for mixed procedures that begin with assembly. For this reason,
compilers that produce mixed code procedures generally start them with
at least one p-code (e.g. NAT, or NOP and NAT), so that Exit_IC will be
defined.

####  Constant pool

Multi-word contants embeded in p-code are pulled out by the compiler and
placed in a dedicated area, just after the last procedure in the
segment. The area is pointed at by Cte_Pool. All p-code instuctions that
use constants refer to them as offsets from the bottom of the code pool.
This strategy allows p-code to be completely byte-sex independent. Of
course, the code pool is not and its bytes will need to be flipped if
the machine on which the program is loaded does not have the same sex
that the machine on which the program was written.

Real numbers in floating point format represent a special case. These
are coded in one of the two canonical format described above. The word
Real_Size in the segment header indicates the [real number
format](#real%20numbers) used by the machine that created the program
(this word is valid even if no real number was actually defined). It
will be 32 for the 3-words format (32 bits) and 64 for the 3-to-6 words
format. Real numbers are clustered together in a separate region of the
constant pool, which is pointed at by the very first word in the pool.
The first word in the real sub-pool is the number of real constants that
follow.

Example:

    | Proc dict  |
    +------------+
    |  Constant  |
    :    ...     :
    |  Constant  |
    +------------+
    |    Real    |
    :    ...     :
    |    Real    |
    +------------+
    | # of reals |
    +------------+<-,
    |  Constant  |  |
    :    ...     :  |
    |  Constant  |  |
    +------------+  |
    |  Sub-pool -|--'
    +------------+ <--- Cte_Pool
    | Last proc  |

####  Relocation list

The relocation list is generally placed after the procedure dictionary.
It is pointed at by the Rel_List word in the segment header. It contains
a list of all addresses that need to be patched when the segment is
loaded in memory, or moved around. Note that only assembly needs a
relocation list, since p-code is address-independent.

The relocation list is made of any number of sublists. Each sublist
consists in a header that specifies the size of the sublist and the type
of relocation required, followed by a list of pointers to the addresses
that need to be patched. Here is the structure of this header:

    List_Header = PACKED RECORD
                   List_Size: integer;
                   Data_Seg_Num: 0..255;
                   Reloc_Type: Loc_Types;
                  END

    Loc_Types = (Reloc_End, Seg_Rel, Base_Rel, Interp_Rel, Proc_Rel);

**List_Size** is the number of pointers that follow the header in the
sublist, i.e. the number of addresses to patch.
**Data_Seg_Num** is only used by sublists of type Base_Rel and is zero
for other types. It contains the local number of the data segment to
which the pointers in the sublist are relative. Its value is normally
assigned by the Linker.
**Reloc_Type** specifies the type of relocation requires. Reloc_End
(value = 0) serves as a terminator to indicated that there are no more
sublists in the relocation list. The Proc_Rel type (procedure-relative)
is produced by the Assembler but will be changed to Seg_Rel
(segment-relative) by the Linker, so you should never encounter it in a
linked code file.

Example:

- +------------+--------------+
      | Reloc_Type | Data_Seg_Num |
      +------------+--------------+ <--- Reloc_List
      | List_Size                 |
      +---------------------------+
      | Pointer                   |
      :  ...                      :
      | Pointer                   |
      +------------+--------------+
      | Reloc_Type | Data_Seg_Num |
      +------------+--------------+
      | List_Size                 |
      +---------------------------+
      | Pointer                   |
      :   ...                     :
      | Pointer                   |
      +------------+--------------+
      |     0      |      0       |
      +------------+--------------+

Note that the relocation list grows from top to bottom: Reloc_List
points at the sublist with the highest offset in the segment, whereas
the sublist with a Reloc_Type of zero (Reloc_End) is at the bottom.

####  Segment reference list

The segment reference list is only needed by Unit segments. It is
necessary because segment numbers are assigned arbitrarily by the
compiler within a compilation unit. The segment reference list
associates these unit-specific numbers the with the corresponding
segment names, so that the OS can set up the proper E_Vect and E_Rec
structures when building up the run-time environment. Once this is done,
the segment reference list is discarded, so it won't use up any memory
at execution time.

The list is placed at the end of the segment, after the relocation list
(if any). Its location can be deduced by using the field Code_Length in
the segment dictionary as an offset from the beginning of the segment.
The size of the list (in words ) is also found in the segment
dictionary, in the word Ref_Size.

The list consists simply in a series of records with the following
structure:

    Seg_Reg = PACKED RECORD
               Seg_Name: PACKED ARRAY[0..7] OF CHAR;
               Seg_Num: 0..255;
               Filler: 0..255;
              END

**Seg_Name** is the 8-character segment name.
**Seg_Num** is the number this segment was assigned in this compilation
unit.
**Filler** is reserved for future extensions.

The end of the list is marked by a record with a Seg_Name consisting in
8 spaces, and a Seg_Num of zero. It could also be calculated from the
Ref_Size value (5 words per record).

Dummy records with Seg_Name "\*\*\*" are generated so that the OS can
execute the initiation and termination code of a unit. When loading a
program, the OS creates a list of all units that contain such a record,
finishing with the main program. It then calls procedure number 1 in the
first unit of the list. Procedure number 1 is reserved for the
initialization/termination section of a unit. When done with
initialization, the first unit calls the next one, which calls the next,
etc, until the last unit calls the main program. When the program
terminates, the \*\*\* list is popped and the same procedures are
returned to, thereby travelling the chain back in reverse order.

####  Linker informations

Code files produced by the Assembler and the Compiler may contain some
information for the Linker to use in linking assembly language segments
to p-code segments. Segments produced by the Assembler always have
linker information, segments produced by the Compiler only have some if
they contain EXTERNAL routines. The Has_Linker_Info flag in the segment
dictionary indicates whether a segment has such information or not.

Linker information is always at the end of the segment, starting on a
block boundary after the segment reference list (if any). There is no
pointer to it in the segment header, but its address can be calculated
from the segment entries in the segment dictionary:
`Code_Addr + ((Code_Length + Seg_Refs + 255) / 256)`.

Linker information is a series of 8-word records, whose contents depends
on the type of linking required. Records are padded with zeros if less
than 8 words are actually used. Some records are followed by a list of
pointers whithin the segment. All records can be defined with a single
Pascal structure, even though they only share the common Name and
LI_Type fields.

    LI_Types = (EOF_Mark, Glob_Ref, Publ_Ref, Priv_Ref, Const_Ref,
                Glob_Def, Publ_Def, Const_Def, Ext_Proc, Ext_Func, Sep_Proc, Sep_Func);

    LI_Entry = RECORD
                Name: PACKED ARRAY[0..7] OF CHARS;
                CASE LI_Type: LI_Types OF
                 Glob_Ref, Publ_Ref, Const_Ref:
                   (Format: (Word, Byte, Big);
                    N_Refs: integer);
                 Priv_Ref:
                   (Format: (Word, Byte, Big);
                    NRefs: integer;
                    N_Words: integer);
                 Ext_Proc, Ext_Func:
                   (Src_Proc: integer;
                    N_Params: integer);
                 Sep_Proc, Sep_Func:
                   (Sec_Proc: integer;
                    N_Params: integer;
                    Kool_bit: boolean);
                 Glob_Def:
                   (Home_Proc: integer;
                    IC_Offset: integer);
                 Publ_Def:
                   (Base_Offset: integer;
                    Pub_Data_Seg: integer);
                 Const_Def:
                   (Const_Val: integer);
                END;

    Ptr_List: ARRAY[0..ceiling(N_Refs/8)] OF
               ARRAY[0..7} OF integer;

Ok, let's take these one at a time. Remember that they all have a Name
field that hold the name of the identifier, and a LI_Type field that
indicated the type of linking required.

**Glob_Ref** is used to link identifiers between assembly routines.

     Glob_Ref:
      (Format: (Word, Byte, Big);
       N_Refs: integer);

**Name** is the name of the identifier (a.k.a. label).
**Format** is always 0 (i.e. "word").
**N_Refs** is the number of references in the list that follows the
record.

**Ptr_List:** The reference list comes by chunks of 8 words. For
Glob_Ref, the Linker will add the final segment offset to each of the
words pointed at by the reference list. The offset could be in words or
in bytes depending on the assembly language used.
**Publ_Ref** is used to link a label in an assembly routine to a
variable in a compilation unit (compiled into p-code).

     Publ_Ref:
      (Format: (Word, Byte, Big);
       N_Refs: integer);

**Name** is both the name of the label and the name of the variable in
the compilation unit.
**Format** is always "word".
**N_Refs** is the number of references in the list that follows the
record.

The linker will add the offset of the variable to all words pointed at
by the reference list.
**Const_Ref** is used to link a label in an assembly routine to a
constant defined in a compilation unit.

     Const_Ref:
      (Format: (Word, Byte, Big);
       N_Refs: integer);

**Name** is the name of the label, and the name of the constant.
**Format** can be "word" (0) or "byte" (1).
**N_Refs** is the number of references in the list that follows the
record.

The linker will place the constant byte or word at all locations
specified in the reference list.
**Priv_Ref** is used to allocate space is the global data segment.

     Priv_Ref:
      (Format: (Word, Byte, Big);
       NRefs: integer;
       N_Words: integer)

**Format** is always "word".
**N_Words** is the number of words to allocate.
**N_Refs** is the number of references in the list that follows the
record.

The Linker will add the offset of the beginning of the allocated area to
each word pointed at by the reference list.
**Ext_Proc** and **Ext_Func** are generated by the Compiler to reference
EXTERNAL routines. They are not followed with a pointer list.

     Ext_Proc, Ext_Func:
      (Src_Proc: integer;
       N_Params: integer);

**Name** is the name of the routine.
**Src_Proc** is the number assigned to the routine.
**N_Params** is the number of words used for parameter passing.
**Sep_Proc** and **Sep_Func** are generated by the Assembler to declare
routines. They are not followed with a pointer list.

     Sep_Proc, Sep_Func:
      (Sec_Proc: integer;
       N_Params: integer;
       Kool_bit: boolean);

**Name** is the name of the routine.
**Src_Proc** is the number assigned to the routine.
**N_Params** is the number of words used for parameter passing.
**Kool_Bit** is true if the routine is relocatable ( created with
`.RELPROC` and `.RELFUNC`) and false otherwise.
**Glob_Def** is generated by the Assembler to declare a global
identifier (with `.DEF`, `.PROC`, `.FUNC`, `.RELPROC` or `.RELFUNC`).
There is no pointer list.

     Glob_Def:
      (Home_Proc: integer;
       IC_Offset: integer);

**Name** is an identifier valid within the segment, that can be
referenced by other routines from the same segment.
**Home_Proc** is the number of the routine in which the identifier is
defined.
**IC_Offset** is the offset in bytes of the label whithin the routine
that contains it.
**Publ_Def** is generated by the Compiler to declare a global variable
that should be visible to `EXTERNAL `routines. It is not followed with a
pointer list.

     Publ_Def:
      (Base_Offset: integer;
       Pub_Data_Seg: integer);

**Name** is the name of the variable.
**Base_Offset** is the offset in words of the variable, from the start
of the segment that contains it.
**Pub_Data_Seg** is the local number of the data segment that contains
the variable.
**Const_Def** is generated by the Compiler to declare a global constant
that should be visible to EXTERNAL routines. It does not have a pointer
list.

     Const_Def:
      (Const_Val: integer);

**Name** is the name of the constant.
**Const_Val** is the value of the constant.
**EOF_Mark** is a special type that marks the end of the linker
information list. **Name** should be filled with spaces, the remainder
of the record, as always, filled with zeros.

Note that not all segments can have every type of linker information.
Compiler generated Prog_Seg and Unit_Seg can :

Refer to Ext_Proc and Ext_Func defined by assembly routines

Define Publ_Def and Const_Def for assembly segments to refer to

And of course, issue an EOF_Mark to mark the end of the list.

Assembly generated Seprt_Seg can:

Define Sep_Proc and Sep_Func for p-code segments to call

Define Glob_Def and refer to Glob_Ref, for assembly-to-assembly linking

Refer to Publ_Ref and Const_Ref to link with p-code

Issue Priv_Ref to reserve space

Issue an EOF_Mark.

###  Assembly files

The structure of a pure assembly file, generated by the Assembler,
resembles a lot the Compiler-generated code files described above. There
are a few differences however.

Most importantly, each procedure has its own relocation list, located
just after the body of the procedure. Since Exit_IC is not used by
assembly routines, this word is used to point at the top of the
relocation list (remembers, it grows downwards). These relocation lists
are the only ones in which the Proc_Rel relocation type can be used. The
Linker will convert all the procedure's lists into a big segment's list,
in which the Proc_Rel will be replaced with Seg_Rel.

For each procedure, the Data_Size word is \>FFFF, which is zero in one's
complement notation. It indicates 1) that no data space is required, 2)
that the first instruction in the routine is an assembly opcode.

Since the Assembler does not know the segment name, all routines that
communicate with REFs and DEFs are assumed to be in the same segment.

And since assembly segments don't know what program or unit they are
linked to, the Prog_Name entry in the segment dictionary is left blank.
For the same reason, the Data_Seg_Num field in the List_Header record of
Base_Rel sublists will contain zeros. The proper value will be filled in
by the Linker. The Linker also updates the Relocatable bit in Seg_Misc
arrays according to what was supplied in the linker information section.

At linking time, it is also possible that the code bodies of assembly
routines will be copied into one of the segments of the compilation unit
they are linked to.

[Next page: the TI-99/4A p-code card](pcode.md)
Revision 1. 9/21/01 Ok to release.

[Back to the TI-99/4A Tech Pages](titechpages.md)

 
