# Assembly language primer

Hey, I said "primer". Not "tutorial", ok?

- [Machine language](#machine-language)
- [Assembly language](#assembly-language)
- [CPU registers](#cpu-registers)
- [Addressing modes](#addressing-modes)
- [Bytes vs words](#bytes-vs-words)

- [Opcodes](#opcodes)
- [Common assembly-time instructions](#instructions)

[Encoding format
](#Encoding%20format)[Hexadecimal notation](#hex%20notation)

## Machine language

Basically, a computer consists of a microprocessor together with some
memory. All the additional gear (keyboard, screen, mouse, etc) is just
here for a secondary purpose: allow a human to interface with the
computer.

The microprocessor is the brain of the computer: it controls the whole
system according to the instructions contained in a program. But no
matter how sophisticated, the microprocessor still does not understand
english! The only thing it understands are numbers. In the case of the
TI-99/4A, numbers from 0 to \>FFFF, i.e. 65535 (if you don't know what I
mean by \>FFFF, thre will be an [explanation](#hex%20notation) of the
hexadecimal notation at then end of this page).

Each operation that the microprocessor can execute has been assigned a
number: \>A000 (40960) for addition, \>6000 for substraction, etc. A
program is nothing else that a carefully arranged list of such numbers.

Actually, I slightly oversimplified the situation. Some instructions are
encoded by a unique number, but most can be encoded by a range of
numbers, depending on their targets. Let's thake the negation operation
for instance: it can be encoded by any number between \>0500 and \>053F.
Each number corresponds to a different target for the instruction:
\>0500 instructs the microprocessor to negate register 0, \>0501 to
negate register 1, etc.

The set of valid numbers accepted by the processor therefore constitues
a language. It is called "machine language". In the case of the
TI-99/4A, the microprocessor (the TMS9900) understands 69 different
instructions, encoded by a total of 57792 numbers (i.e. some numbers do
not correspond to any valid instruction).

##  Assembly language

As you probably realise, machine language is not very convenient for a
human to use. Who's gonna recall 57792 numbers and know what they are
used for? It is possible to program directly in machine language: I can
do it to some extent, to patch a program with a sector editor for
instance. But this is only convient for tiny bits of program using a
small subset of instructions. If we want to do anything more complex,
we'll need an assembler.

An assembler is a program that translates human managable mnemonics into
machine language. For instance, the mnemonic for the addition is "A",
the one for substaction is "S" and the one for negation is "NEG". Such
mnemonics are called "opcode" for operation code.

Another set of mnemonics can be combined with the opcodes to define the
target of an instruction (i.e. its operand). For instance, register 0
would be designated as "R0" and register 1 as "R1".

To negate register 0 you would write:
Similarly, to add R1 to R0:
It's easier to handle than machine language isn't it?

This set of opcodes and operands is called "assembly language". It's
nothing else than a litteral representation of machine language in a
human-managable form.

Of course, since we are using an assembler, we could as well add some
extra features that will make our life easier. The assembler may
automatically check for syntax errors and tell us about it. It may also
allow us to replace numbers with alphanumeric "labels". For instance, I
could define a label for the number 10:
EQU means "equates" and is not an opcode, in that it will not be
translated to machine language. It is an assembly-time instruction, used
to tell the assembler that we would like to use the word "TEN" as a
synonym for the number ten. Wherever we write "TEN" in our program the
assembler will replace it with 10 before it translates the program into
machine language:

       LI   R0,10        And loads the value 10 into R0

Whereas opcodes and operands are pretty much defined by the structure of
the microprocessor, assembly instructions are only limited by the
programers imagination. Therefore, different assemblers may used very
different instructions. In all examples you'll find on this website, I
have used the instruction set of the original assembler released by
Texas Instruments with the module "Editor/Assembler".

#### Object files

Conceivably, an assembler could load machine code directly into memory
at then end of the assembly process. But that would not be convenient
because you would have to re-assemble a program every time you want to
execute it. Therefore most assemblers are file-based programs.

The TI assembler takes its input from a Dis/Var 80 text file and
produces a Dix/Fix 80 object file in tagged-object format. It also
offers you the option to produce a list file, where all syntax error
will be reported: this is easier than trying to jot them down as they
flash on screen!

The object file contains machine language mixed with special
instructions for the linker and the loader (the 'tags'). If you are
curious, the tagged-object format of this file is described in my
[page](ea_mod.htm#tagged-object) on the Editor/Assembler cartridge.

#### Linker

A linker is a program that lets you piece together several object files
assembled independently. This is a nice feature when you write really
big assembly programs, because it means that you don't have to
re-assemble a mammoth program every time you make a small change. You
can split the program into smaller files and assemble them separately,
then you only re-assemble the one you modified.

Also, you could have a library of usefull routines, whether written by
you or not, and link them to your program as independently assembled
files. This saves you the need to reinvent the wheel for every program.

Finally, some linkers will let you link assembly with other languages,
such as GPL or Extended Basic.

A very good linker is the RAG linker, from R.G. Green. It takes one or
more DF80 object files and produces an executable version of the
program, in the form of a memory-image "program" file.

The memory-image file must then be loaded into memory by yet another
program, called a loader. The Editor/Assembler cartridge contains such a
loader in its option 5, which is why memory-image files are often
refered to as EA5 files. The TI-writer option 3 also calls an EA5
loader, so do Funnelweb options 1 to 3. Finally, you will find a
stand-alone loader on this site: my
[MILD](download.htm#MILD%20and%20MISS) loader that let's you load both
machine language and GPL.

Texas Instruments also came up with several hybrid "linking-loaders"
which you'll find in Editor/Assembler option 3, Mini-memory option 1 and
Extended Basic CALL LOAD. As their name implies, these programs perform
linking while the program is loaded. This is much slower then loading
EA5 files, but it has the advantage that object files can be loaded
anywhere there is room in memory, whereas EA5 files are meant to run at
a predefined location. Also, linking with Extended Basic is easier this
way.

Once the program is in memory, you can use a small utility called SAVE
to dump it into an EA5 program file.

In summary, the process of creating assembly language programs can be
represented as such:

    Your brain
         |
         | Editor
         |
         V
    Text File (DV80)
         |
         | Assembler
         |
         V
    Object file (DF80)  Other object files (DF80)
         |                |
         | Linker         |
         |                |
         V                V
    Memory-image EA5 file (Program)
         |
         | EA5 loader
         |
         V
    Program is executed in memory

Alternatively, using the TI Editor/Assembler cartridge:

    Your brain
         |
         | Editor
         |
         V
    Text File (DV80)
         |
         | Assembler
         |
         V
    Object file (DF80)   SAVE utility (DF80)
         |                    :
         | EA3 linking loader :
         |                    :
         V                    V
    Program is executed in memory
         :
         : SAVE is called (optional)
         :
         V
    Memory-image file (Program)

#### Source file format

The format of the source file is the only one we are concerned with for
the moment being, since this is the file that you must write.
Fortunately, its format is very simple: each line in the file contains
an opcode with its operands, generally in the form:

    [label] Opcode [operand][,operand] [comments]
    Spaces ^      ^                   ^

The fields in \[brakets\] are optional. Note however that the number of
operands is determined by the opcode and is therefore not optional for a
given opcode: some have no operands, some have one, some have two. The
syntax is a little more relaxed for instructions to the assembler, and
some of these can take a variable number of operands.

Labels in the left margin are used to easily refer to a location in your
program, for instance to branch to it. There must be at least one space
between label and opcode, between opcode and operands, and between
operands and comments (if any). If there are two operands, they should
be separated with a comma, not with spaces.

Comments are very important with assembly language, since it can be
quite difficult to figure out what a program is doing when reading the
source file. This even applies to your own programs: you'd be surprised
how difficult it is for you to understand what you wrote just a few
month ago! Therefore, use comments a lot: leave messages for yourself.

If you need more room than just the end of the line, you can enter lines
that contain only comment. Just make sure that such lines begin with a
\* that will instruct the assembler to ignore them.

Example:

    * This is a comment line
    MYTEST MOV  R1,R2    This is a sample assembly language line

####  Minimal assembly program

So what's the smallest assembly program that you can write? Something
like this:

    START  RT          return to caller
           END  START

**`RT`** is not a real opcode, it's an assembly alias for one of the
most common return statement, which should actually be written `B *R11`
This statement returns to the caller, in this case the loader that will
return control to you (hopefully).

**`END `**is an instruction that tells the assembler that the end of the
file has been reached. An optional operand can also be used to indicate
the entry point of your program: in our case this is the label START.

If you don't specify an entry point, an EA5 loader will begin execution
at the top of your program. An EA3 loader will wait for you to enter the
name of a label where you want to start execution. So you can do this,
you must tell the assembler to include the label into the object file.
This is done as follows:

           DEF  START
    START  RT          return to caller
           END

The **`DEF `**instruction makes one or more label available to the
linker and/or the loader.

##  CPU Registers

Before we discuss assembly language further, I would like to introduce
you briefly to the structure of the TMS9900 microprocessor (a.k.a. CPU
for "Central Processing Unit).. You'll find a more complete description
in [here](tms9900.md) in case you are interested.

First let's talk about registers. A register is nothing else than a
memory cell that is integrated inside the microprocessor. The advantage
is that it's much easier and faster for the microprocessor to access a
register than to access the external memory. The TMS9900 has 3 main
registers:

[The program counter
](#PC)[The workspace pointer
](#WS)[The status register](#ST)

####  Program counter

The Program Counter (PC) keeps trace of the currently executed
instruction. This allows the TMS9900 to fetch the next instruction to be
executed. Some instructions allow you to load a different value inside
PC, thereby performing a "jump" or "branch" inside your program (the
equivalent of a Basic GOTO). The program counter is a 16-bit register
that simply contains the memory address of the current instruction.

####  Status register

The Status register (ST) contains flag bits use by the TMS9900 to make
decisions. Most operations affect one or the other of these bits. And
conversely, some instructions behave differently according to the status
of a given bit (e.g. conditional jumps). The structure of the status
register is the following:

|     |      |     |     |       |     |     |     |          |                |
|-----|------|-----|-----|-------|-----|-----|-----|----------|----------------|
| Bit | 0    | 1   | 2   | 3     | 4   | 5   | 6   | 7 to 11  | 12 to 15       |
| Use | High | GT  | Equ | Carry | Ovf | Par | Xop | not used | Interrupt mask |

The first bits deal with mathematical operations:

**High** means logically higher than, for unsigned operations. For
instance, you could use the compare opcode "C" to compare R0 and R1:

      LI  R1,&gt;FFFF     Loads number 65535 (a.k.a. -1) into R1
      C   R1,R0        Compares R1 to R0

After the comparison instruction, the "High" bit will be set to 1 since
\>FFFF is logically higher than 1.

**GT** (greater than) performs the same function, but consider that the
operands contain signed values. By convention, \>FFFF means -1, \>FFFE
means -2, \>FFFD means -3, ... and \>8000 means -32768.

In the above example, GT will be 0 since -1 is arithmetically smaller
than 1.

**Eq** means equal and is set by the comparison operation when the two
operands contain equal values.

And now, let's mention a very usefull feature of the TMS9900: after
almost each operation that deals with numbers the processor will
aurtomatically compare the result to zero and set the status bits
accordingly. For instance, if you decrement R0 by 1:
the content of the status register will reflect the result of the
comparison of R0 (after it was decremented) with zero. If you were using
R0 as a countdown counter, there is no need to use a "C" instruction to
compare it to zero: this was already done by the DEC instruction.

**Carry** is used to indicate a carry over. It can be viewed as a 17th
bit, left of bit 0. For instance, if you do:

      LI  R1,&gt;FFFF     Load number 65535 (a.k.a. -1) into R1
      A   R1,R0        Add R1 to R0

The result will be \>10000, which is too big to fit in 16 bits. R0 will
therefore contain \>0000 and the "Carry" bit will be set to indicate
that the value of R0 "wraped over".

**Ovf** indicates an overflow during some operations. Mostly, ovf deals
with the sign bit (i.e. bit 0). For instance, if you add two positive
numbers and the result may be understood as a negative number, ovf will
be set:

      LI  R1,&gt;4001     Load number into R1
      A   R1,R0        Add R1 to R0

The result of the addition (to be placed in R0) is \>8003, wich is
perfectly correct for unsigned values: 16384+16385 = 32769. However, if
we were to consider these numbers as signed, we would get: 16384+16385 =
-32767. The TMS9900 sets ovf to warn us about a potential problem in
this case, as well as in similar situations.

**Par** stands for parity. It is only used by a limited number of
opcodes. The microprocessor checks all the bits involved in these
operations and counts how many of them were "1". If the result is odd,
the Par bit will be set. This can serve as a transmission error control
mechanism for instance, but is generally of little use to the average
programmer.

**Xop** is set during the execution of a [XOP](#XOP) instruction.

The **interrupt masks** tells the TMS9900 upto which priority level it
can accept interrupts. In the TI-99/4A console, all interrupts are
hardwired at level 2. Therefore only two cases are of importance for us:

If the interrupt mask is 0 or 1, interrupts are not allowed

If the interrupt mask is 2 to 15, interrupts are allowed.

In case you wonder, an interrupt is a hardware-triggered event that
causes the TMS9900 to temporarely stop execution of the current program
and to execute another program instead: the interrupt service routine
(ISR). Once the ISR is completed, the TMS9900 will (hopefully) resume
execution of the program where is was interrupted.

####  Worskpace pointer

Registers are also meant to be used by the programmer: since they are
located on chip they are faster than external memory. Furthermore, since
there is only a limited number of registers, they can be encoded within
the instruction number itself (cf the example above for NEG R0 and NEG
R1). By contrast, if we wanted to negate an address in the external
memory, we wourld write:
But now we need 16 bits to specify the address of the word to negate,
obiously it cannot fit together with the opcode into a 16-bit
instruction word! Therefore, this instruction will require an extra word
and will be encoded as: \>0520 \>2000 (the \>0020 in \>0520 indicates
that there is an extra word in this instruction).

With instructions that deal with two operands (like the addition) it may
be necessary to have an extra word for each of them. Therefore a TM9900
instruction can be 1, 2 or 3 words long.

Now how many registers are there for us to use inside the TMS9900? Well,
none... or 16.... or as much as we want!

Let me explain that. Instead of placing registers inside the
microprocessor, the TI designers decided that registers will be in the
external memory. The TMS9900 only contains a workspace pointer, i.e. a
register that contains the address of 16 pseudo-registers in the
external memory.

This kind of defeats the principle of a register: it will take as much
time to access a register that to access a memory address... since
registers are memory addresses. Well, not quite. We saw above that an
instruction requires an extra word to use an operand in the external
memory. Thus, by using a register we make the program shorter, and we
also make it faster: we save the time that would be required to read
that extra word.

In addition, if we change the value of the workspace pointer, we
automatically start to work with a fresh set of 16 registers. If we
change it back to what it was before, we'll found our previous registers
with their original content. This is very usefull for context switching,
i.e. to switch from one task to another. This is what occurs during the
execution of an interrupt for instance: the TMS9900 automatically
switches to a workspace placed at \>83C0 and "remembers" where was the
workspace the main program was using. This allows to return from the
interrupt with an undisturbed set of registers.

You can also cause such a context switch programmatically, using
instructions like BLWP (Branch and Load a new Workspace Pointer), LWPI
(Load Workspace Pointer from Immediate value) and RTWP (ReTurn using old
Workspace Pointer).

###  Addressing modes

If you followed the above, you will have now understood that a given
instruction address memory in two different ways: as a register (i.e. on
offset with respect the the workspace pointer) or as an absolute
address. These different ways are known as "addressing modes". There is
a total of eight addressing modes for the operands of the TMS9900
instructions, but out of these three are reserved for sets of special
instructions.

[Immediate operands](#Immediate): e.g. `LI R0,12`
[Register addressing](#register%20addressing): e.g. `CLR R1`
[Indirect register addressing](#Indirect%20register): e.g. `CLR *R1`
[Auto-incrementing addressing](#auto-increment): e.g. `CLR *R1+`
[Direct addressing](#direct): e.g. `CLR @>2000`
[Indexed addressing](#indexed): e.g. `CLR @>2000(R1)`
[PC relative addressing](#PC%20relative%20adressing): e.g. `JMP $+4`
[CRU relative addressing](#CRU%20addressing): e.g. `SBO 0`

####  Immediate operands

These only apply to a restricted set of instructions. An immediate value
is a number that immediately follows the instruction in the program.
This value is to be used as an operand.

We already encountered one of these: the `LI `instruction that appeared
in many examples above:
In machine language this corresponds to:
\>0200
\>0001.

Where \>020x encodes the LI opcode, \>xxx0 encodes register 0 and the
extra word \>0001 encodes the immediate value.

The "immediate" opcodes are:

- **`LI`**` R1,123 `` `(load immediate value)
- **`AI`**` R1,-12 `` `(add immediate value)
- **`ANDI`**` R1,>1FFF `` `(and immediate value)
- **`ORI`**` R1,>8040 `` `(or immediate value)
- **`CI`**` R1,25 `` `(compare with immediate value)
- **`LWPI`**` >83E0 `` `(load worskpace pointer with immediate value)
- **`LIMI`**` 2 `` `(load interrupt mask with immediate value)

As you can see, they all end with letter I (and no other opcode does).
They are thus pretty easy to recognize and to remember.

Most have two operands. The first one must be a register (no indirect,
no indexing, just a plain register addressing). LWPI and LIMI have only
one operand because the register is implicitly defined by the
instruction: the Worspace pointer register and the Status register of
the TMS9900, respectively.

The second operand must be an immediate value: it's a requirement, not a
choice.

By contrast, other opcodes generally have the choice between the 5 other
addressing modes. There are a few exceptions to this rule though: some
opcodes can only deal with registers for on apparent reason. That's
because the TI designers ran out of numbers to encode all these
possibilities in machine language.

####  Register addressing
This one is already an old friend: the operand is simply one of the
sixteen registers in the current workspace. They are abbreviated R0
through R15.

Note that the "R" is optional with the TI assembler. In fact, to use the
"R" you must set a special option when launching the assembler. I didn't
know that when I learned assembly (using a hacked assembler, with no
manual) and I therefore got the habit of not using the R. I had a hard
time remembering to add it while I was typing the examples for these
pages...

####  Indirect register addressing
Here the register is not the final target of the instruction. Instead,
it contains the address of the target (i.e. it is a pointer to it).

In the above example, if R0 contains \>A123 the instruction will result
in negating word at address \>A123.

This allow you to operate on different addresses according to the
situation, even though the exact address could not be predicted by the
time you wrote the program.

####  Indirect auto-incrementing register addressing
These is almost exactly the same as the above, with one exception: once
the instruction is completed, the content of R1 will be incremented (by
two in this case, because we are dealing with words).

That comes extremely handy for copy loops for instance:

```asm
       LI   R2,&gt;A000      Into that one
       LI   R0,&gt;1000      That's the number of bytes to copy
L1     MOV  *R1+,*R2+     Copy one word
       DECT R0            We decrement by two because 1 word is 2 bytes
       JGT  L1            DECT automatically compares R0 with 0.
*                         If it's greater we jump to L1, other wise we come here
```

Do you see how it works?

- The MOV instruction copies the content of the address to be found in
  R1 (in this case \>2000) to the address to be found in R2 (in this
  case \>A000). Then is automatically changes R1 to \>2002 and R2 to
  \>A002.
- The next execution of the loop will copy \>2002 to \>A002 and change
  R1 to \>2004 and R2 to \>A004.
- The next one copies \>2004 to \>A004, etc

####  Direct addressing

A.k.a. Symbolic addressing
That one is easy to understand: you just use the memory address of the
word you want to target. Optionally, you could designate this word with
a label, hence the name "symbolic":
Finally, most assembly will let you enter arithmetic expressions like:
The assembler calculates the value of COUNTER+4 (in our case \>2004) at
assembly time and encodes the instruction as `NEG @>2004. `One more
example of the usefull things an assembler can do for you.

####  Indexed addressing
This one kind of combines `@>2000` with `*R1`: the target address is
calculated by adding the direct (a.k.a. symbolic) address and the
content of R1.

For instance, if R1 contains 2, the above instruction negates \>2002.
If R1 contains \>0548, it negates \>2548 etc.

Note that R0 cannot be used as an index (this is because `NEG @>2000` is
in fact encoded as `NEG @>2000(R0)`, the TMS9900 knows that in this case
it must not use R0).

This adressing mode is usefull in two situations:

      NEG  @TABLE(R1)     Negates entry 8 in the table

In this case, the symbolic part of the operand refers to a table of
values you placed in memory. R1 represents an index inside that table
(hence the name of this addressing mode). Of course, we could have
written `NEG @TABLE+8`, but in the above example, the content of R1 can
vary. Therefore, the NEG instruction can apply to one or the other word
in the table, according to the value of R1.

The second situation is the "mirror image" of the first one:

      NEG   @8(R1)      Negates entry 8 in the list

It does the same job as the above: which one you want to use is mainly a
matter of taste. Sometimes you prefer to think "I'm targetting the 8th
entry in the table" and sometimes you like better "I'm dealing with the
8th byte after the current position in the list".

There are two more addressing modes that are only used by well defined
sets of opcodes (just like immediate operands are).

####  PC relative addressing

This one is used exclusively by jump instructions:
**`JMP `**(unconditional jump)
**`JEQ `**(Jump if Equal, i.e. if the Equ bit is 1 in the status
register)
**`JNE `**(Jump if Not Equal, i.e. if the Equ bit is 0)
**`JGT `**(Jump if Greater Than)
**`JH`** (Jump if Higher)
**`JHE `**(Jump if Higher or Equal)
**`JL `**(Jump if Lower, i.e. if the "High"and the "Equ" bits are 0)
**`JLE `**(Jump is Lower or Equal)
**`JLT `**(Jump if Less Than, i.e. if the "GT" and the "Equ" bits are
0)
**`JNC `**(Jump if No Carry)
**`JOC `**(Jump On Carry)
**`JNO `**(Jump is No Overflow)
**`JOP `**(Jump on Odd Parity, i.e. if the "Par" bit is set)

Again these are easy to recognise: they all begin with "J" (no other
opcode does).

Youn will note however that there are some missing: what about
`JLTE `and `JGTE`? And why isn't it a "jump if overflow" and a "jump on
even parity". Again that's because the machine language is running out
of valid numbers.

But the missing one are the logical complement of existing instructions:
Therefore, to perform the equivalent of "Jump if Less Than of Equal" you
could invert the comparison and use "`JGT`":

      JGTE SK1       Error: &quot;illegal opcode&quot; `
      C    R4,R1     Do the inverse comparison
      JLT  SK1       Now this is gonna work

Similarly, you could compensate for a defficient "jump if overflow" by
changing the structure of your program:

       JIO  SK1      Jump if overflow? Does not exist!
       MOV  R4,R2    Continue if no overflow
       ...
SK1    NEG  R1       Do something if overflow occured`
       A    R1,R4    Let&#39;s try again
       JNO  SK1      Jump if no overflow
       NEG  R1       React to overflow here
       ...
SK1    MOV  R4,R2    And continue here if no overflow

The main advantage of jump instructions, is that they can be encoded
together with the opcode. How is this possible? Didn't I just mentionned
that a memory address uses up 16 bits and therefore always requires an
extra word in the program?

Yes, but you see jump to not use memory addresses. Instead they jump to
a given distance from the current instruction: plus or minus 127 words.
This allows to encode all jump values with a single byte and leaves the
other 8 bits for the opcode. BUT it means that we cannot jump everywhere
in the program: the destination address must be within the 127 words
limit (actually it's 128 words forwards and 127 backwards). Here again,
the assembler wil calculate this for you and issue an error message if
the jump cannot be encoded. This "out of range" message is one of the
most frequent you will encounter...

There are two ways to specify a jump in assembly. First you can type the
displacement yourself:

      JLT  $+4      Jump 4 bytes down if R1 is less than R0 (signed)
      JMP  $-120    Jump 120 bytes up otherwise (replaces a JGTE )

The \$ sign stands for "current value of the program counter". The
assemble then knows that the offset you specified is relative to the
current instruction. (FYI: in machine language jumps are WORDS relative
to the NEXT instruction: therefore a JMP \$+4 is encoded as \>1001, not
as \>1004. But the assembler takes care of this for you).

In the above example, it's relatively easy to figure that `JLT $+4`
jumps over the `JMP`. But who wants to count 120 bytes upwards to figure
out where the `JMP `is going to land?

Therefore, the assembler lets you use a label to specify the
destination. The value of this label is most simple defined by placing
it in the left margin of the target instruction: the assembler
automatically assigns the current value of the program counter (i.e. the
memory location of the opcode) to such labels.

      JLT  UHOH       And another one here
      ...             Continue here if both are negative
OK    MOV  R1,R2      Here if first test hit
      ...
UHOH  NEG  @&gt;2000     And here if second jump taken

Note that contrarily to braching instructions, you must not include the
@ sign before a label in the case of a jump (as opposed to `B @OK`).
This is usefull to remind you that you are restricted to a limited range
of motion.

####  CRU relative addressing

The CRU (Communication Register Unit) is a special hardware trick that
allows the TMS9900 to communicate with pieces of equipment without using
the data bus. Conceptually, it could be viewed as a set of 2048 wires
linking the microprocessor to peripherals (in fact it uses only three
lines, plus the address bus, but that's not the point).

You can also view it as a set of 2048 bits, one per each "wire". By
reading a bit you can test the status of a wire, by writing to a bit you
can change the status of a wire. Provided of course that the peripheral
in question is designed to allow these operations: there could be
read-only bits, write-only bits and not-used bits!

There are five CRU operations:

**`TB `**(Test Bit: transfer the bit into the Equ bit of the status
register)
**`SBO `**(Set Bit to One)
**`SBZ `**(Set Bit to Zero)
**`LDCR `**(Load CRU: allows to modify upto 16 bits at a time)
**`STCR `**(Store CRU: allows to read upto 16 bits at a time)

      SBO  7             Turn light on
      TB   7             Read it back
      JEQ  OK            Jump if bit is 1 (not zero)

Note that TB is somewhat missleading: JEQ jumps it the bit is 1 and JNE
if the bit is zero. That's because the bit is copied in the Equ bit
(that normally stands for "equals zero" in automatic comparisons).

So what's the use of the `LI R12,>1300` instruction? Well, instead of
numbering the bits from zero, the TMS9900 numbers them from the value
found in R12, divided by two (because the last addess line does not
exist on a word-oriented microprocessor, we cannot encode uneven
addresses).

That's usefull when dealing with peripherals: each card is assigned a
chunk of 128 bits at addresses starting at \>1000, \>1100, \>1200, etc
upto \>1F00 (addresses \>0000 to \>0FFF are internal to the console). If
you want to turn on the disk controller card, whose CRU address is
\>1100 you may do:

     SBO   &gt;880         That&#39;s &gt;1100 divided by two

But that's kinda hard to remember right.

Now you could do the same with:

      SBO  0            By convention, the first bit of a card turns it on

Which is somewhat easier to use: the bits are relative to the card, not
to the whole CRU address space

And when it comes to LDCR and STCR, you don't even have a choice: you
must use the second style since you can't specify the starting bit in
the instruction (it's always zero)

      LDCR R1,3          Load 3 CRU bits from R1 into the disk controller card

Because \>1102 accesses bit 1 of the disk controller card (remember: the
content of R12 is the bit number times 2), the above example affects the
values of bit 1, 2, and 3 of the controller card.

Note that with LDCR and STCR the only valid addressing mode is register
(that's one of those case where numbers were missing...). The bits are
always taken/stored from the right to the left of the register. If the
operation deals with 1 to 8 bits, only the left byte of the register
will be used: a byte-oriented operation is assumed. With 9 to 15 bits,
the whole register is accessed.

For a more complere explanation of the CRU, see [here](cru.md).

##  Bytes versus words

The TMS9900, and the TI-99/4A that is built around it, is a
word-oriented, byte-addressable processor.

What this means is that the processor mainly deals with 16-bit words.
But that these words are stored in a memory that is addressed as bytes.
It may seem silly, but that's what happened in most computer systems:
even a 32-bit Pentium is still using a byte-addressable memory.

With 16 bits we can encode numbers from 0 to 65535, which means that the
size of the memory has to be at most 64K bytes if we want to be able to
deal with an address as a single value. The memory could be twice as
large is it were word-addressable, but that's not the case.

For the processor, it means that the least significant address line (A15
in the TI numbering convention) is perfectly useless: it just represents
the byte inside the word. Therefore, the TMS9900 has only 15 address
lines: A0 through A14.

Nevertheless, the CPU can deal with byte values if necessary. It just
uses an internal trick: when you write a value to a byte, the processor
reads the whole word, modifies the byte you were accessing, and writes
the word back. There is a bunch of opcodes that allow such byte-oriented
operations: AB, SB, MOVB, CB, SOCB, SZCB and in some cases LDCR and
STCR.

When the operand is a register, the only byte that you can access in
this manner is the leftmost one, the most significant. That's the byte
stored in memory at an uneven address. When using direct addressing, you
can specify an odd or an even address indifferently for byte operations.

On the other hand word operations always require an even address, since
the 16-bit value is transfered as such on the data bus (and the address
is always even since A15 is missing).

      S    R1,@&gt;2000    Substracts &gt;0104 from &gt;2000-2001
      S    R1,@&gt;2001    Ditto! It does not affect &gt;2002   !!!
      SB   R1,@&gt;2000    Substract &gt;01 from &gt;2000. &gt;2001 unchanged
      SB   R1,@&gt;2001    Substracts &gt;01 from &gt;2001. &gt;2000 unchanged

Just remember: for word operations always use an even address.

####  Data bus multiplexing

For some mysterious reason, the TI-99/4A designers decided to cripple
their machine by using an 8-bit data bus to access most peripherals.
Only the console ROMs and the 256 bytes of console RAM known as the
"scratch-pad" are accessed via a 16-bit data bus.

For the rest of the memory, a special circuitery in the console
multiplexes the data bus and passes it as twice 8 bits. This requires
the creation of an extra address line, A15, which indicates which byte
is currently accessed. We therefore end up with a 16-bit address bus.
Note that A15 is multiplexed with the CRU data output line, but this is
not a problem during memory operations since this CRU line will be
inactive.

The big drawback is that memory access is now twice slower, and the
multiplexer circuit has to put the TMS9900 on hold until the transfer is
completed!

##  Opcodes

[Branching opcodes
](#branching)[Arithmetic opcodes
](#Arithmetic%20opcodes)[Bitwise logic opcodes
](#logic%20opcodes)[Shift opcodes
](#Shift%20opcodes)[CRU opcodes
](#CRU%20opcodes)[Various opcodes
](#Various%20opcodes)[Forbidden opcodes](#Forbidden%20opcodes)


&gt;0000-01FF, &gt;0320-033F,
&gt;0780-07FF,&gt;0C00-0FFF

### Branching opcodes

These opcodes allow you to control the flow of your program, by jumping
from a point to another, possibly remembering where to come back.

#### Jxx *offset*

We already discussed the [jump](#PC%20relative%20adressing) opcodes.
They are very usefull since they are conditional: your program can make
decisions and follow one or the other path of execution according to the
value of a given status bit.

####  B *dest*

The first one is very easy to understand: B just loads the value of the
destination operand into the program counter register of the TMS9900. As
a result, the program execution continues from that address. Note that
contrarily to jumps there is no range limitation for B: the whole 64K
range is within reach. This is because the address is passed as an extra
word after the opcode.

Status bits affected: none

####  BL *dest*

Branch-and-Link does exactly the same thing, except that it remembers
where it came from: the address of the next instruction (the one that
would be executed of the branch were not taken) is placed in register
R11. Then the program execution continues from the specifed address: in
this case it is marked by a label called "THERE".

Status bits affected: none

How to return from such a branch? Very easy: just use a B instruction
with an R11 addressed as an indirect operand. Like that:

      RT

Here we are just telling the TMS9900: branch at the address that you
will find in R11. Which happens to be the return address, since BL
automatically placed it in here.

Of course, you should carefully preserve this return address. If you
plan to use R11, make sure you first transfer its content into another
register or memory location. A very common mistake is to place another
BL within a function called with a BL: the new return point will
overwrite the old one.

BACK  NEG   R0`
HERE  BL    @THERE      Incorrect example: branch at &quot;THERE&quot; and place &quot;DONE&quot; into R11
DONE  ABS   R0
      B     *R11        This returns at &quot;DONE&quot; since its now the value in R11
THERE B     *R11        This returns at &quot;DONE&quot;`
* Now here is the correct example
      BL    @HERE       Branch at &quot;HERE&quot;, place address &quot;BACK&quot; into R11
BACK  NEG   R0`
HERE  MOV   R11,R10     Save return point
      BL    @THERE      Branch at &quot;THERE&quot; and place &quot;DONE&quot; into R11
DONE  ABS   R0
      B     *R10        This returns at &quot;BACK&quot; since its the value in R10
THERE B     *R11        This returns at &quot;DONE&quot; since its the value in R11   

####  BLWP *dest*

Branch-and-Load-Workspace-Pointer is slightly trickier: here we want to
branch, but also to change the workspace. To be able to return to the
calling point, we must not only remember the return address (i.e. the
content of the PC register), but also the location of the old workspace
(i.e. the content of the WR register). And since we are at it, we may
also record the value of the status register. This way, when we come
back, we can restore the situation just like is is now.

You may think that BLWP is called with two operands, one for the address
to branch at, the other for the new workspace. If that's the case, you
came close but no cigar! BLWP has only one operand, a pointer: it
contains the address of a pair of words that contain the address to
branch at and the new workspace. It is more flexible this way since you
only have to know one value (the pointer) to perform the branch.

WHERE DATA  &gt;8300       New workspace to be used
      DATA  HERE        Address to branch at`
HERE  MOV   R0,R1       The BLWP branches here

So where are the three values we wanted to remember upon branching? The
pointer to the old worskpace is automatically placed into R13, the
return address (the instruction that follows the `BLWP`) into R14, and
the current value of the status register into R15.

Here also, we should be carefull not to overwrite them with our data.
However, unlike BL, there is no risk of erasing them with a second
`BLWP`, since that one will save its return values into R13-R15 of the
NEW workspace. That's a very neat feature: it means that `BLWPs `can be
nested as deep as we want! The drawback is that this operation is fairly
slow, as the TMS9900 must do a lot of memory transfer to complete it...

Status bits affected: none

####  RTWP

Now how do we return to the caller and restore the three TMS9900
registers? There is a dedicated instruction for that purpose:
Return-with-Worskpace-Pointer. Upon execution it will branch to the
address found in R14, with the workspace specified in R13 and place the
content of R15 in the status register. And we just have restored the
situation as it was before the `BLWP `was taken.

That's an ideal feature for a multi-tasking operating system. Too bad
there isn't one around... Actually, there is one multi-tasking feature
in the TI-99/4A: the interrupt service routine. Upon reception of an
interrupt, the TMS9900 performs an implicit `BLWP @>0004`, i.e uses the
values found at \>4000 and \>4002 in the console ROM as workspace
(\>83C0) and address (\>0900). At this address you'll find the interrupt
service routine, the subprogram that handles all the interrupts. This
subprogram returns to the main program with a simple `RTWP `and the main
program never knows it was interrupted.

Know, if you have an "assembly freak" mind, you'll probably have thought
of an alternative use for `RTWP`: change the content of the registers in
the TMS9900. All we have to do is to load values into R13, R14 and R15
and execute a `RTWP`.

      LI   R14,THERE    This will be the new address
      MOV  R1,R15       The content of R1 will be the new status
      RTWP              Do it`
      LWPI &gt;83E0        We could alter the workspace this way
      B    @THERE       We could change the address this way
*     ???               But there is no opcode to load the status

This trick is often used to return at a differnt address in case an
error occured within a procedure: just place the address of the error
handling routine into R14 and the next `RTWP `will branch to it. It is
good practice to save the previous value of R14 somewhere, in case the
error handling routine would need it, after all

####  XOP

Extended operations are implicit `BLWPs `that use vectors located at
precise addresses in the console ROMs:

XOP 0 uses addresses \>0040 and \>0042 (worskpace \>280A, address
\>0C1C)
XOP 1 uses addresses \>0044 and \>0046 (workspace \>FFD8, address
\>FFF8)
XOP 2 uses addresses \>0048 and \>004A (values may vary according to the
ROM version), etc

XOPs constitute an advanced feature and are discussed in the TMS9900
[page](tms9900.htm#XOP).

####  Parameter passing

While we are taking about branching and calling subroutines, I would
like to briefly discribe the three classical ways for a subroutine to
get parameters from the calling program: global variables, registers and
immediate data.

* to a subroutine called with BL`
      CLR   @COUNTER     Place the value &gt;0000 into the word at address COUNTER
      LI    R0,&gt;01B4     Place value &gt;01B4 into R0
      BL    @SUB1        Call a subroutine
      DATA  &#39;BA&#39;         Place the value &#39;BA&#39; (&gt;4241) into a data word
BACK  MOV   R1,R0        Continue execution here `
SUB1  MOV   @COUNTER,R2  Use the value in COUNTER: global parameter
      A     R0,R2        Use the value in R1: parameter in register
      MOV   *R11+,R0     Get the value in the data word. AND change R11 to return to BACK
      LI    R1,&gt;0456     To return a result: put it in a register
      MOV   R2,@RESULT   Our into a global

It's not considered as good programming practice to make to much a wide
use of global parameters. You should reserve them for critical values
that are needed by many subroutines in your program: using the as global
avoids having to pass them with a register every time.

Whether to use a register or a data word largely depends on what kind of
information is passed by this parameter. If it can contain a wide range
of values that vary often, better use a register. If it just allows to
differentiate between a few situation uses a data word: this will save
you a register, and a word in memory (the one occupied by the LI
instruction in the caller).

For instance: a routine that places a character somewhere onscreen
should probably pass the character and the screen position in registers.
This will make it more versatile. By contrast, a routine used to display
pre defined error messages may well use dat words as there won't be that
many messages and they will probably always be displayed at the same
address.

Note the way the data word is retrieved in the subroutine: it
automatically increments R11 by two, which will skip the data word upon
return

The corresponding operations for a subroutine called with `BLWP `would
be the following:

* to a subroutine called with BLWP`
      CLR   @COUNTER     Place the value &gt;0000 into the word at address COUNTER
      LI    R0,&gt;01B4     Place value &gt;01B4 into R0
      BLWP  @SUB4        Call a subroutine, this time changing the context
      DATA  &#39;BA&#39;         Place the value &#39;BA&#39; (&gt;4241) into a data word
BACK  MOV   R1,R0        Continue execution here `
SUB4  DATA  WREGS,SUB40  Use this workspace and this address`
SUB40 MOV   @COUNTER,R2  Use the value in COUNTER: global parameter
      A     *R13,R2      Use the value in old R0: parameter in register
      MOV   *R14+,R0     Get the value in the data word. AND change R14 to return to BACK
      LI    R1,&gt;0456     To return a result: put it in a register
      MOV   R1,@2(R13)   Remember: the address of the old workspace is in R13
      MOV   R2,@RESULT   Our return value in a global

Did you get the trick with R13? Since R13 contains the address of the
old workspace, `*R13` points at the old R0, `@2(R13)` at the old R1,
`@4(R13)` at the old R2, etc.

####  Returning values

If the subroutine wants to return a value, it can place it in a
register, or in a global variable. Generally, one does not return values
in a DATA statement (because the program could be placed in ROM, in
which case the data word won't take the new value). On the other hand,
the following trick is often use to indicate a special condition upon
return:

      JMP  ERROR       In principle, returns here
      MOV  R0,R1       In fact, returns here if no error occured `
SUB2  CI   R0,&gt;0111    Compare R0 to &gt;0111
      JL   ERR         We don&#39;t want it to be smaller (silly example)
      INCT R11         Change return address: skip the JMP if no error
ERR   B    *R11        Return at the address found in R11
     

SUB2 indicates to the caller that everything went OK by skipping the JMP
upon return. Another way to do it would be to place the test in the
caller, since the B instruction does not affect the status register:

      JL   ERROR       Use the comparison performed in SUB3
      MOV  R0,R1      `
SUB3  CI   R0,&gt;0111    Compare R0 to &gt;0111
      B    *R11        Let the caller do the conditional branch
     

Now what about routines called with `BLWP`? The first solution is
identical, just replace R11 with R14:

      JMP  ERROR       In principle, returns here
      MOV  R0,R1       In fact, returns here if no error occured `
SUB8  DATA WREGS,SUB80 New workspace and branching address`
SUB80 CI   R0,&gt;0111    Compare R0 to &gt;0111
      JL   ERR         We don&#39;t want it to be smaller (silly example)
      INCT R14         Change return address: skip the JMP if no error
ERR   BLWP             Return at the address found in R14
     

The second solution is trickier since the RTWP discards the content of
the status register and replaces it with R15. We therefore have to store
the status in R15 beforehand (or some value of our choice).

      JL   ERROR       Use the comparison performed in SUB9
      MOV  R0,R1      `
SUB90 DATA WREGS,SUB80 New workspace and branching address`
SUB80 CI   R0,&gt;0111    Compare R0 to &gt;0111
      STST R15         Store the status register in R15
      RTWP             Return with the new status taken from R15
     

Finally, let me remind you that you can change the return address by
loading any value in R11 (or R14 for `RTWP`)

BACK   ABS  @TABLE(R2)     That will return here
      `
ENTRY2 LI   R11,ERROR      Special entry point: always return to error
ENRTY1 MOV  R0,R0          Regular entry point: return provided by BL
       B    *R11 

###  Arithmetic opcodes

The following opcodes can be used to perform integer math. Unless
otherwise indicated, the source and destination operands can be accessed
in any of the five main addressing modes: Rx, \*Rx, \*Rx+, @xxxx and
@xxxx(Rx).

#### MOV *source,dest*

Copies the content of the source operand into the destination operand
and compares it to zero.
Status bits affected: High, Gt, Equ

####  MOVB *source,dest*

Same as MOV, but affects only the leftmost (most significant) byte of
the operands.
Status bits affected: High, Gt, Equ, Parity

####  LI *register,immediate*

Loads the immediate value into the destination register and compares it
to zero.
Status bits affected: High, Gt, Equ

####  A *source,dest*

Adds the content of the source operand to the destination operand and
compares the result to zero.
Status bits affected: High, Gt, Equ, Carry, Ovf

####  AB *source,dest*

Same as A, but affects only the leftmost (most significant) byte of the
operands.
Status bits affected: High, Gt, Equ, Carry, Ovf, Parity

####  AI *register,immediate*

Adds an immediate value to the destination register.
Status bits affected: High, Gt, Equ, Carry, Ovf

NB. There is no SI (substract immediate) instruction, but you can use AI
with negative values: `AI R0,-5`

####  S *source,dest*

Substracts the content of the source operand from the destination
operand and compares the result to zero.
Status bits affected: High, Gt, Equ, Carry

####  SB *source,dest*

Same as S, but affects only the leftmost (most significant) byte of the
operands.
Status bits affected: High, Gt, Equ, Carry, Ovf, Parity

####  C *source,dest*

Compares the content of the source operand to that of the destination
operand.
Status bits affected: High, Gt, Equ

####  CB *source,dest*

Same as C, but affects only the leftmost (most significant) byte of the
operands.
Status bits affected: High, Gt, Equ, Parity

####  CI *register,immediate*

Compares the register to an immediate value.
Status bits affected: High, Gt, Equ,

####  DEC *dest*

Decrements the destination operand by 1 and compares the result to
zero.
Status bits affected: High, Gt, Equ, Carry, Ovf

####  DECT *dest*

Decrements the destination operand by 2 and compares the result to
zero
Status bits affected: High, Gt, Equ, Carry, Ovf, Parity

####  INC *dest*

Increments the destination operand by 1 and compares the result to
zero
Status bits affected: High, Gt, Equ, Carry, Ovf

####  INCT *dest*

Increments the destination operand by 2 and compares the result to
zero
Status bits affected: High, Gt, Equ, Carry, Ovf, Parity

####  NEG *de*st

Negates the destination operand and compares the result to zero.
Status bits affected: High, Gt, Equ, Carry, Ovf, Parity

####  ABS *de*st

Takes the absolute value of the destination operand (i.e. negates it if
it is less than 0) and compares the result to zero.
Status bits affected: High, Gt, Equ, Carry, Ovf, Parity

NB There are no DECB, DECTB, INCB, INCTB, NEGB, nor ABSB byte-oriented
operations.

####  MPY *source,register*

Multiplies the source operand and the destination register. The result
will probably be 2-word long and will therefore be placed into the
destination register and the following register! (If R15 is the
destination, the next memory word after the workspace is used).

Example: R0 \* R1 --\> \[R1-R2\]

```asm
      LI   R1,10
      MPY  R0,R1  Now R1-R2 contains 10
*                 i.e. R1 is 0, R2 is 10
```

####  DIV *source,register*

Divides the 2-word value found in the destination regiser and the
following register by the content of the source operand. The result is
placed in the destination register. The remainder in the next
register.
Status bits affected: Ovf

Example: \[R1-R2\] / R0 --\> R1, Remainder in R2

```asm
      LI   R1,8
      LI   R2,3   R1-R2 contains &gt;0008 0003
      DIV  R0,R1  Now R1 is &gt;0002
*                 and R2 is &gt;0003 (remainder)
```

###  Bitwise logic opcodes

The following operations deal with operands on a bitwise basis, i.e. the
operands are not considered as words or bytes but as a collection of
individual bits. The change of a given bit will never affect
neighbouring bits.

####  CLR *dest*

Reset (clears) all bits in the destination operand to zero. The result
is \>0000.
Status bits affected: none

####  SETO *dest*

Sets all bits in the destination operand to one. The result is \>FFFF.
Status bits affected: none

####  INV *dest*

Inverts all bits in the destination operand (logical NOT) and compares
the result to zero.
A 0 bit becomes a 1, a 1 bit becomes a 0.
inv 0 = 1
inv 1 = 0

Status bits affected: High, Gt, Equ

####  ANDI *register,immediate*

Ands the bit in the destination register with those in the immediate
value and compares the result to zero.
If both the source and destination bit are 1, the resulting bit will
be 1. Otherwise it will be 0.
0 andi 0 = 0
0 andi 1 = 0
1 andi 0 = 0
1 andi 1 = 1

Status bits affected: High, Gt, Equ

####  ORI *register,immediate*

Ors the bit in the destination register with those in the immediate
value and compares the result to zero.
If both the source and destination bit are 0, the resulting bit will be
0. Otherwise it will be 1.
0 ori 0 = 0
0 ori 1 = 1
1 ori 0 = 1
1 ori 1 = 1

Status bits affected: High, Gt, Equ

####  XOR *source, register*

Exclusive OR of the bits in the source operand with those in the
destination register. The result is compared to zero.

If a bit is 1 in either the source or the destination operand, but not
both, it will be one. Bits that are identical in both operands are reset
to 0:
0 xor 0 = 0
0 xor 1 = 1
1 xor 0 = 1
1 xor 1 = 0

Status bits affected: High, Gt, Equ

NB There is no XORI instruction

####  SOC *source,dest*

Sets to 1 all bits in the destination operand that correspond to a 1 in
the source operand and compares the result to 0. The other bits are
unchanged.

Set Ones Corresponding is therefore the equivalent of a logical OR:
0 soc 0 = 0
0 soc 1 = 1
1 soc 0 = **1  **
1 soc 1 = **1**

Status bits affected: High, Gt, Equ

Example:

      LI   R1,&gt;1021
      SOC  R0,R1     R1 now contains &gt;1421

####  SOCB *source,dest*

Same as SOC, but affects only the leftmost (most significant) byte of
the operands.
Status bits affected: High, Gt, Equ, Parity

####   SZC *source,dest*

Resets to 0 all bits in the destination operand that correspond to a 1
in the source operand and compares the result to 0. The other bits are
unchanged.

Set Zero Corresponding is therefore the equivalent of a logical NOT-A
AND B:
0 szc 0 = 0
0 szc 1 = 1
1 szc 0 = **0  **
1 szc 1 = **0**

Status bits affected: High, Gt, Equ

Example:

      LI   R1,&gt;1420
      SZC  R0,R1     R1 now contains &gt;1020

####  SZCB *source,dest*

Same as SOC, but affects only the leftmost (most significant) byte of
the operands.
Status bits affected: High, Gt, Equ, Parity

####  COC *source,dest*

Checks whether all bits in the destination operand that correspond to a
1 in the source operand are 1. Sets the Equ bit if this is the case. All
bits are unchanged.

Compares Ones Corresponding therefore performes a masked comparison with
\>FFFF:
0 coc 0 --\> not considered
0 coc 1 --\> not considered
1 coc 0 --\> Equ will be 0 if this happens at least once
1 coc 1 --\> ok so far.

Status bits affected: Equ

Example:

      LI   R1,&gt;1021
      LI   R2 &gt;4481
      COC  R0,R1     Equ = 0 (because &gt;0400 is not set in R1)
      JEQ  SK1       The jump is not taken
      COC  R0,R2     Equ = 1
      JEQ  SK2       The jump is taken

####  CZC *source,dest*

Checks whether all bits in the destination operand that correspond to a
1 in the source operand are 0. Sets the Equ bit if this is the case. All
bits are unchanged.

Compares Ones Corresponding therefore performes a masked comparison to
with \>0000:
0 coc 0 --\> not considered
0 coc 1 --\> not considered
1 coc 0 --\> ok so far
1 coc 1 --\> Equ will be 0 if this happens at least once.

Status bits affected: Equ

Example:

      LI   R1,&gt;1021
      LI   R2 &gt;4080
      CZC  R0,R1     Equ = 0 (because &gt;0001 is not reset in R1)
      JEQ  SK1       The jump is not taken
      CZC  R0,R2     Equ = 1
      JEQ  SK2       The jump is taken

NB These is no COCB, nor CZCB byte-oriented instruction.

###   Shift opcodes

The following operations shift the content of a registed, i.e. move all
bits in it towards the left or the right.

Shifting one position to the left corresponds to a multiplication by two
(just as moving the decimal point by one position corresponds to a
multiplication/division by 10). Shifting to the right corresponds to a
division by two. However, in the latter case we may have to take the
sign bit into account, if we are dealing with signed values: therefore
there are two shift-right instructions: one for logical operands, one
for arithmetic operands.

For each you can specify by how many positions you want to shift the
bits: legal values are 1 to 15. Shifting by 0 has of course no effect,
so this value is interpreted differently: If you specify a shift by 0,
the number of positions is taken from the value in R0 (rounded to 15).
If this value is zero, the bits will be shifted by 16 positions.

####  SLA *register,count*

Shifts the content of the register by *count* positions to the left,
filling up the bits on the right with 0. The result is compared to zero,
the last bit shifted out is placed in the carry bit.

Status bits affected: High, Gt, Equ, Carry

NB There is no SLL (Shift Left Logically) because it's equivalent to SLA
(Shift Left Arithmetic).

Example:

      SLA  R0,1          R0 now contains &gt;0802
      SLA  R0,4          R0 now contains &gt;8020
      SLA  R0,1          R0 now contains &gt;0040 and the Carry is 1

####  SRL *register,count*

Shifts the content of the register by *count* positions to the right,
filling up the bits on the left with 0. The result is compared to zero,
the last bit shifted out is placed in the carry bit.

Status bits affected: High, Gt, Equ, Carry, Ovf (=1 if sign bit is
changed).

Example:

      SRL  R1,1          R1 now contains &gt;4200 and Carry is 1
      SLA  R1,4          R1 now contains &gt;0420 and Carry is 0
      LI   R0,1          Shift by one
      SLA  R1,0          R1 now contains &gt;0210 (shift value taken from R0)

####  SRA *register,count*

Shifts the content of the register by *count* positions to the right,
filling up the bits on the right with copies of the sign bit. The result
is compared to zero, the last bit shifted out is placed in the carry
bit.

Status bits affected: High, Gt, Equ, Carry

Example:

      SRA  R1,1          R1 now contains &gt;C001 (i.e. -16383, correct division by 2)
*     SRL  R1,1          By contast, a SRL would give &gt;4001, which is 16385!

####  SRC *register,count*

Shifts the content of the register by *count* positions to the right,
filling up the bits on the left with those ejected on the right. The
result is compared to zero, the last bit shifted out is placed in the
carry bit.

Status bits affected: High, Gt, Equ, Carry

NB There is no SLC (Shift Left Circular), but you can do it with SRC
(Shift Right Circular): just substact the desired displacement from 16:
`SLC R1,5` is encoded by `SRC R1,11`.

Example:

      SRC  R1,1          R1 now contains &gt;8200 and Carry is 1
      SRC  R1,4          R1 now contains &gt;0820 and Carry is 0
      SRC  R1,12         R1 now contains &gt;8200 (equivalent to SLC R1,4)

###  CRU opcodes

These opcodes affect the CRU. See above for the CRU [addressing
mode](#CRU%20addressing). I also have a whole [page](cru.htm#software)
that discusses CRU issues. You may want to have a look at it.

#### SBO *bit*

Sets a CRU bit to 1. The bit number can be from 0 to 15 and is relative
to the CRU address in R12.
Status bits affected: none

####  SBZ *bit*

Sets a CRU bit to 0. The bit number can be from 0 to 15 and is relative
to the CRU address in R12.
Status bits affected: none

####  TB *bit*

Tests a CRU bit. The bit number can be from 0 to 15 and is relative to
the CRU address in R12. The bit is copied into the Equ bit in the status
register.
Status bits affected: Equ

####  LDCR *register,nbits*

Loads *nbits* bits to the CRU, starting from the CRU address in R12. If
there are 1 to 8 bits, they bits are taken (from right to left) from the
most significant byte of the source register. If there are 9 to 16 bits,
they are taken (from right to left) from the whole register.

Status bits affected: High, Gt, Equ, Carry. (Parity for 1-byte
operations)

####  STCR *register,nbits*

Reads *nbits* bits from the CRU, starting from the CRU address in R12.
If there are 1 to 8 bits, they bits are stored (from right to left) into
the most significant byte of the source register. If there are 9 to 16
bits, they are stored (from right to left) into the whole register.

Status bits affected: High, Gt, Equ, Carry. (Parity for 1-byte
operations)

###  Various opcodes

Finally, here are some opcodes that I couldn't fit in any of the above
categories.

#### LWPI *immediate*

Loads an immediate value into the workspace pointer register of the
TMS9900, effectively changing the workspace. Just make sure that
*imme*diate does not correspond to an address in ROM !

Status bits affected: none

####  STWP *register*

Stores the content of the workspace pointer register of the TMS9900
(i.e. the address of the current workspace) into the destination
register.

Status bits affected: none

NB. There is no LDWP, so if you want to change the value of the
workspace you have two solutions:

* We have two solutions:`
* 1) Put the value in our program and perform a LWPI`
       MOV  R0,HERE+2     Replaces the &gt;0000 below with our value
HERE   LWPI &gt;0000         Loads the new workspace pointer
     `
* 2) Put the value in R13 and perform a dummy RTWP`
       STST R15           Make sure the status won&#39;t be affected
       LI   R14,CONT      Make sure we go on normally
       MOV  R0,R13
       RTWP
CONT   ...                Execution continues here with our new workspace

####  STST *register*

Stores the status register of the TMS9900 into the destination register.

Status bits affected: none

NB There is no LDST to load a value in the status register. But you can
put it in R15 and perform a RTWP (just make sure that R13 contains the
proper workspace, and R14 a valid address).

* Solution: Put the value in R13 and perform a dummy RTWP`
       STWP R13            Make sure thje workspce won&#39;t change
       LI   R14,CONT1      Make sure we go on normally
       MOV  R0,R15
       RTWP
CONT1  ...                 Execution continues here with new status

####  LIMI *immediate*

Loads a value from 0 to 15 into the interrupt mask part of the status
register. Only interrupts with a level equal or smaller than this value
will be recognized. On the TI-99/4A all interrupts are hardwired as
level 1. Therefore:

LIMI 0 and LIMI 1 enable interrupts
LIMI2 to LIMI 15 disable interrupts

Status bits affected: interrupt mask

####  SWPB *dest*

Swap the most significant and the least significant bytes of the
destination operand.

Status bits affected: none.

Example:

       SWPB R1           R1 now contains &gt;2BA5

####  X *dest*

Executes the machine language instruction whose value is found in the
destination operand. If the instruction requires additional memory words
for operands, they will be taken after the X instruction, not after the
address of the operand.

Status bits affected: depends on the instruction executed.

Example (also see the [page](tms9900.htm#X) on the TMS9900)

       LI   R9,&gt;045B     This means B  *R11
       X    R9           Do it`
* This performs: LI   R1,&gt;1234
       LI   R0,&gt;0201     This means LI  R1,xxxx
       X    R0
       DATA &gt;1234        Value to load in R1`
* This performs: NEG  @&gt;2000
       X    @TEST        Opcode to execute
       DATA &gt;2000        Operand used`
TEST   NEG  @&gt;0000

###  Forbidden opcodes

There are five opcodes that should never be used with the TI-99/4A,
because they would be mistaken for CRU operations. These are:

CKON
CKOF
LREX
RSET
IDLE

See the TMS9900 [page](tms9900.htm#External) for details.

##   Assembly-time instructions

Besides opcodes, you can also include instructions to the assembler
whithin your source file. These instructions may or may not generate
code, some deal with a printout feature, some issue commands for the
linker, etc.

They may vary from assembler to assembler, therefore I only included
here the most common ones.

Instuctions that generate data: [DATA](#DATA), [BYTE](#BYTE),
[TEXT](#TEXT) or reserve room for it [BSS](#BSS), [BES](#BSS), [EVEN
](#EVEN)Instructions for the linker: [DEF](#DEF), [REF
](#REF)Instructions for the loader: [AORG](#AORG), [RORG](#RORG), [END
](#END)Others: [COPY](#COPY)

####  DATA *value*\[*,value*\]

This instuction is used to generate code that is not an opcode.
Generally, to place data in your program.

*value* can be any number (or math expression) between 0 and \>FFFF. It
can also be entered as a 2-character string constant. If needed, many
data words can be place on the same line, separed with comas.

Note that a data word will always be loaded at an even address, on a
word boundary.

Example:

       DATA &#39;TE&#39;

####  BYTE *value*\[*,value*\]

Pretty much like `DATA`, except that it generates only one byte, and is
therefore not limited by word boundaries.

*Value* can be any number between 0 and 255, or a single-character
string.

####  TEXT '*string*'

This instruction is used to insert data into your program. The data is
specified in the form of a quoted string.

Example:
####  BSS *nbytes* BES *nbytes*

These two instructions are used to reserve space into your program.
Generally you will place data there. Contrarily to the above
instructions, `BSS `(block starting with symbol) and `BES `(block ending
with symbol) don't place values into memory: they just tell the loader
to skip *nbytes* before to load the next instruction.

`BSS `only differs from `BES `when you use a label: the label value
corresponds to the current address with `BSS `and to the current address
plue *nbytes* with `BES `(i.e. the end address).

Example

       LI   R2,256     Buffer size in bytes
L1     CLR  *R1+       Clear a word in it
       DECT R2         Decrement bytes counter
       JNE  L1         More to clear
       B    *R11`
BUFFER BSS  256        Reserve 256 bytes of memory`
       MOV   R0,R1     Next procedure

####  EVEN

Is used to make sure the loading will continue from a word boundary.
This instruction is usefull afer a `TEXT `statement, when you don't want
to count the number of characters to know whether you must add a
`BYTE `statement or not. `EVEN `issues a \>00 data byte if the current
address is uneven, it does nothing otherwise.

####  DEF *label*\[*,label\]*

This instructions is used to pass one or more labels to the loader. The
values of these labels will thereby be available for other files, or for
the loader itself (to launch you program for instance).

*label* must be a valid label (1 to 6 characters, the first one being
not a digit) and must be defined in the source file.

Example:

      DEF COUNT`
START MOV  R11,R10       Entry point of my program
      ...                Long program
      B    *R10          Return to the caller`
INIT  CLR  @COUNT        Procedure available to another file
      B    *R11  `
COUNT DATA 10            Data word available to another file`
     END

####  REF *label*\[*,label*\]

This is the mirror-image of `DEF`. It allows your source file to
reference labels that are part of another source file. You can thereby
access its variables, call its procedures, etc.

You are not allowed however to perform arithmetic on REF labels since
these are undefined at assembly time

Example:

       BL   @INIT           Call a procedure in another file
       MOV  @COUNT,R0       Use a data word from another file
       MOV  @COUNT+2,R1     Illegal: no math allowed with REF labels
       LI   R1,COUNT        Instead, do this
       MOV  @2(R1),R1       It&#39;s ok, since the math is done at execution time

####  AORG *address*

This instruction will force the loader to load the program at a precise
memory location. In general you won't use it, except to modify a precise
data word. E.g. to place values in the non-maskable interrupt vectors at
\>FFFC-FFFF.

####  RORG \[*offset*\]

This instruction allows the loader to determine itself where the program
should be loaded. The TI loader starts from \>A000 towards \>FFD8, and
continues with the low-memory expansion (in which there is very limited
space, since that's were the loader is located, and it also contains a
symbol table for the REFs and DEFs).

`RORG `can be used to cancel the effect of an `AORG`.

If an offset is specified, it will be added to the current address. The
effect is similar to that of the `BSS `instruction, except that you
could specify a negative offset, thereby causing the loader to overwrite
what it just loaded. (Sounds like a silly thing to do but it may be
usefull with paged-memory devices).

####  END \[*label*\]

This is the only instruction that must be part of any program. It tells
the assembler to stop processing the source file.

If an optional label is specified, it will be used by the loader to
automatically start the program once this file is loaded. Otherwise, you
must DEFine a label and enter its name when the loader asks you where to
start.

####  COPY "*filename*"

This allows for writing very long programs. It instructs the assembler
to switch to the file specified in double-quotes. Once this file is
completely assembled, the assembler will resume with the current one.
Generally, assemblers don't let you nest `COPY `statements, i.e. you
cannot have a `COPY `in a copied file. But you can have as many
`COPY `as you want in the initial file.

##   Encoding format

Assembly language opcodes and operands are encoded into machine language
according to nine fundamental formats. Each uses up a word of memory per
opcode, possibly together with one or two extra words for operands.


Opcode
Td
destination (reg)
Ts
source (reg)
Opcode
PC-relative offset in words
Opcode
register
Ts
source (reg)
Opcode
nbits
Ts
source (reg)
Opcode
count
register
Opcode
Ts
source (reg)
Opcode
 0   0   0   0   0
Opcode
register
Opcode
register
Ts
source (reg)

**Ts** and **Td** define the type of addressing for the source and
destination operand respectively.

00: Rx
01: \*Rx
10: @yyyy(Rx) or @yyyy if Rx = 0 This requires an additional memory word
to store the yyyy value
11: \*Rx+

**Source** and **dest** contain the workspace register, used in the way
indicated be the addressing mode.

**Immed** operands also require an additional word to store the
immediate value.

##  Hexadecimal notation

#### Decimal notation: base 10

In almost all cultures throughout the ages, the numbering system is
based on 10 digits. This is of course because we have 10 fingers (finger
is "digitum" in latin...). The only exception I know of are that Mayas
who used 20 digits. Guess why?

In our culture these ten digits are represented by ten symbols derived
from arabic: "0", "1", "2", "3", "4", "5", "6", "7", "8", and "9".

With 10 digits we can represent ten numbers: zero through nine. Now what
if we want to write down a larger number? Well, we just combine two
digits: one for the tenth and one for the units. 23 means "two times
ten, plus three", which is twenty-three. Similarly, we can add a third
digit for the undredth, a fourth for the thousands, etc.

####  Hexadecimal notation: base 16

But nothing prevents us to use more (or less) than 10 digits. Let's say
we want to use sixteen digits instead of ten. First we need to find
names and symbols for the extra six. We could come up with goofy names
(borg, spam, taku, etc) and fancy symbols, but we wouldn't be able to
type them from a computer keyboard. Therefore let's keep it simple and
decide that the extra digits will be "A", "B", "C", "D", "E" and "F"
(lower case may or may not be ok). In our new system, "A" has the value
ten, "B" is eleven, etc upto "F" which is fifteen.

To represent numbers greater than fifteen, we must combine two digits:
one for the "sixteenth" and one for the units. \>23 means "two times
sixteen, plus three", which is thirty-five. This way we can write down
16\*16=256 numbers.

To go further, we need a third digit whose value will be 256, a fourth
one whose value will be 4096 (i.e. 16\*16\*16), etc.

For instance: \>123B means "1 times 4096, plus 2 times 256, plus 3 times
16, plus 11" which is 4667.

####  Base indication

We also need a way to distinguish our new numbering system, that we'll
call "hexadecimal", from the good old decimal one. Obviously, any number
containing digits from "A" through "F" has to be hexadecimal. But if I
write 10, do I mean ten or sixteen?

Texas Instruments adopted the following convention: any hexadecimal
number must be preceded by a "greater sign" sign. E.g. \>1234

Most PC people use another convention: append a h to the number: 1234h
(which also allows to append a d for decimal numbers, a b for binary,
etc).

High-level languages like C and C++ use yet another convention: start
any hexadecimal number with "0x". E.g. 0x1234. The "x" stands of course
for hexadecimal and the leading 0 is only here so that the compiler
won't mistake the number for a variable name (variable names cannot
start with a digit from 0 to 9 in C/C++).

####  Binary notation: base 2

As mentionned above, we could also use less than 10 digits, if we wanted
two. Let's say we use only two: "0" and "1". This will allow us to write
down the numbers zero and one. To go further we'll need an extra digits
for the pairs: 10 means "one pair, plus no unit" wich is two. And 11
means "one pair, plus one unit" which is three.

The next digits will a a weithg of 4, the next one a weight of 8, etc.

For instance, the number 11001101 reads as "1 times 128, plus 1 times
64, plus 0 times 32, plus 0 times 16, plus 1 times times 8, plus one
times 4, plus 0 times 2, plus 1" which is 205 in decimal.

Why would we want to use such a clumsy numbering system? Because it's
easy to translate in terms of hardware: 0 means "no current" and 1 means
"current flowing" for instance. Or 0 could mean "0 volts" and 1 mean "5
volts".

That's how today's computers are built: they represent all numbers in
binary and encode them as two voltage levels. In the early times of
computing, there were also "anolog" computers, that were using 10
different voltage levels to encode decimal numbers. But these were
technically difficult to built and were completely superceeded by
digital computers.

####  Base conversions

As you have probably noted from the examples above, converting a hex
number into a decimal number is not easy, especially with very large
numbers (You have 30 seconds to calculate the decimal value of
\>123456789ABCDEF0).

The problem is even worse with binary numbers: how much is
10010110101101001001010 in decimal?

Converting decimal numbers to another base is also annoying: How much is
3333 is hexadecimal? We must do the following:

How many times \>1000 (i.e. 4096) in 3333? Zero.
How many time \>0100 (i.e. 256) in 3333? Thirteen, which is \>D in hex
notation. The remainder is 3333-(13\*256)=5.
How many times \>0010 (i.e. 16) in 5? Zero.
How many times \>0001 (i.e. 1) in 5. Five.
The result is thus: \>0D05

By contrast, conversions between hexadecimal and binary are very easy.
Do you see why? Because 16 is a power of 2, whereas 10 is not. Just
compare the "weight" of the digits in the various bases:

Decimal: 1, 10, 100, 1000, 10000, etc.
Hexadecimal: 1, 16, 256, 4096, 65536, etc.
Binary: **1**, 2, 4, 8, **16**, 32, 64, 128, **256**, 512, 1024, 2048,
**4096**, 8192, 16384, 32768, **65536**, etc.

Now do you see the pattern? Every fourth binary digit has the same
weight than an hexadecimal digit (this never occurs with decimal). This
means that we can split any binary number in groups of four digits and
convert them individually into an hex digit.

To come back to the example above (how much is 10010110101101001001010
in hexadecimal?). Piece of cake:

    100 1011 0101 1010 0100 1010 is
    >4   B    5    A    4    A    i.e. >4B5A4A

And conversely, to translate \>1234 in binary:

     >1    2    3    4
    0001 0010 0011 0100 Done in five seconds!

All you need to know are the 16 first binary values. That's easier to
memorize than the powers of two!

|     |        |     |        |
|-----|--------|-----|--------|
| Hex | Binary | Hex | Binary |
| \>0 | 0000   | \>8 | 1000   |
| \>1 | 0001   | \>9 | 1001   |
| \>2 | 0010   | \>A | 1010   |
| \>3 | 0011   | \>B | 1011   |
| \>4 | 0100   | \>C | 1100   |
| \>5 | 0101   | \>D | 1101   |
| \>6 | 0110   | \>E | 1110   |
| \>7 | 0111   | \>F | 1111   |

And this is why computer people use hexadecimal a lot. Note that we
could also have used another power of two as a base. Eight for instance,
using only digits "0" through "7". This is known as "octal" and is
sometimes used, but much less often than hexadecimal. It has the
advantage than you do not need extra digits.

####  Hex math

To perform hexadecimal operations, we'll follow the exact same rules
than for decimal operations:

      >1234
    + >96FB
Lets start from the rightmost digit: "4" plus "B" (four plus eleven) is
"F" (sixteen):

      >1234
    + >96FB
          F
Second digit: "3" plus "F" (three plus fifteen) is eighteen (\>12). We
thus have a carry of sixteen and put down two.

        1
      >1234
    + >96FB
         2F

Now, "2" plus "6", plus the carried "1" is "9".

      >1234
    + >96FB
        92F

And finally "9" plus "1" is "A". Et voila!

      >1234
    + >96FB
      >A92F

We can do the same thing in base 2:

                 1       11       1
     100010   100010   100010   100010   100010   100010
    +010110  +010110  +010110  +010110  +010110  +010110
          0       00      000     1000    01000   101000
              carry 1  carry    report
                       again    carry
I'll let you do substractions as an exercise...

####  Negative numbers

It's often necessary to deal with negative numbers. Therefore several
conventions have been established to represent a negative number in
binay format. Generally, the leftmost bit is used a a sign bit: e.g."0"
means positive and "1" means negative.

The remaining bits may represent the number, and they do in some
conventions. However, the most common convention is "two's complement".
It is the one used by Texas Instruments for the TI-99/4A.

In two's complement notation, negative numbers are represented as
follows:

\>FFFF is -1
\>FFFE is -2
\>FFFD is -3
...
\>8001 is -32767
\>8000 is -32768

The big advantage of this notation is that \>FFFF is greater than
\>FFFE, which is mathematically correct (-1 is greater than -2), and
also true for unsigned numbers.

Problem only occur when comparing a negative number with a positive one:
\>FFFF is greater than \>0001, but -1 is smaller than 1.

That's why the TMS9900 status register contains two status bits for
number comparisons: "high" that deals with unsigned values, and "Gt"
that considers the values as signed.
Revision 1. 6/9/99 Preliminary
Revision 2. 6/13/99 Ok to release
[Back to the TI-99/4A Tech Pages](titechpages.md)
