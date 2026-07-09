# Calling GPL routines from assembly

[The subroutine stack
](#Sub%20stack)[The CALL / RTN mechanism
](#CALL%20RTN)[The DSWGR / RTGR mechanism
](#DSWGR%20RTGR)[The \>0010 and \>0012 routines](#%3E0010%20%3E0012)

The first thing to consider is that there are different types of
routines in GPL:

Routines called with CALL and returning with RTN or RTNC

Routines called with DSWGR and returning with RTGR

Routines called by name with routine G&\>0010 and returning with routine
G&\>0012

All three make use of the subprogram stack to save their return
parameters, so let's first talk about this stack.

###  The GPL subroutine stack

This stack has to be located in the scratch-pad RAM, between addresses
\>8300 and \>83FF. Upon normal GPL execution, the stack starts at \>8380
and grow upward to \>839F.

A stack pointer is maintained in byte \>8373. Since this pointer is only
one byte, the most significant byte of the address is assumed to be
\>83, which is why the stack must be in the scratch-pad. The pointer
points at the last entry saved on the stack. Entries are 2-byte in
length and can be assumed to be word-aligned.

          +---------+
    >83FE :         :
          : (free)  :
          :         :
          +---------+
    >8388 |         |<-,
          +---------+  |
          |         |  |
          +---------+  |
          |         |  |
          +---------+  |
          |         |  |
          +---------+  |
    >8380 |         |  |
          +---------+  |
                       |
          +---------+  |
    >8372 | xx | 88 |--'
          +---------+

Calling instructions save parameters such as the return address on the
stack, after incrementing the pointer. Return instructions fetch the
proper return parameters from the stack, then decrement the pointer.

There is no safety mechanism to detect stack overflow (i.e. overwriting
the 'data' stack that begins at \>83A0), nor underflow (i.e. fetching
meaningless bytes from under \>8380).

##  The CALL and RTN mechanism

The GPL opcode `CALL `(opcode \>06) gets the current address from the
GROMs, saves it on the stack, then branches to the subroutine specified
in the code flow:
Unfortunately, no provision is made to pass the address as a variable.
To do so you would have to resort to complicated tricks like: 1) save
the return address on the stack, 2) save the desired address on the
stack, and 3) perform a dummy `RTN`.

The called routine can return with either `RTN `(opcode \>00) or
`RTNC `(opcode \>01). The difference being that `RTN `clears the Cnd bit
(the GPL equivalent of the Eq bit) in the GPL status byte, whereas
`RTNC `leaves it intact. This gives the caller a chance to test the
result of a subroutine:

       BS   G@SET
       BR   G@NOTSET  

NB If the called subroutine (\>62A5) returned with `RTN`, the `BR `will
always be taken.

Note that no mechanism is provided to return with the Cnd bit always
set. But you can easily do it with:

       RTNC

Since a byte is always equal to itself, the Cnd bit will be set. We're
just wasting memory for the 3 bytes required to encode the
`CEQ `instruction...

###  Calling from assembly: the easy way

If you have a cartridge like Editor/Assembler or Mini-Memory, you have a
very simple way of calling GPL routines: the GPLLNK subprogram. Its
syntax is very simple:

       DATA &gt;6234              GROM address of the routine to call

The subroutine returns after the `DATA `statement, the Cnd bit will
unfortunately be lost in the process and cannot be recovered, even from
\>837C directly.

Also, there is no provision made to pass parameters in the form of data
statements following the call. For instance, in GPL you could write:

       BYTE  &gt;F5,&gt;A2,&gt;83 

The called subroutine would then retrieve the three data bytes with
FETCh instructions. `FETC `looks up the return address on the stack,
fetches bytes from GROM and updates the return address accordingly.
Obviously this is not going to work from assembly, since `FETC `always
accesses GROM, not cpu memory.

###  Calling from assembly: the hard way

Assuming you don't have a cartridge with a GPLLNK routine, you will need
to do the call yourself, which is easy, and to implement a return
mechanism, which is not.

The first thing to do to emulate a call is to pass a return address on
the stack. This will be the address of a GPL routine that enters
assembly at the address you want (e.g. your return address). We'll talk
about that later, let's first see the calling mechanism:

       MOVB @&gt;8373,R1             Get stack pointer
       SRL  R1,8                  Make it LSB
       LI   R0,&gt;xxxx              GROM address of our GPL return routine
       MOV  R0,@&gt;8300(R1)         Save it on the stack

Now that we have set a return address, we can branch to a GPL routine:

       LI   R0,&gt;6234             GROM address of the GPL routine to call
       MOV  R0,@&gt;0402(R9)        Set the address
       SWPB R0
       MOV  R0,@&gt;0402(R9)
       LWPI &gt;83E0                GPL workspace
       B    @&gt;006A               Enter GPL interpreter  

### Returning to assembly: the easy way

If you have a GRAM card and can write your own GPL routines, it is a
simple matter to write one that will return to a predetermined address
in your assembly program. You just use the `XML `instruction. This
instructions gets the address where to enter assembly from 16 tables
located at various places in memory. Each table can hold upto 16
vectors, i.e. addresses where to branch.

The `XML `instruction (code \>0F) is followed by a byte of data whose
first nibble indicates the table, and the second nibble indicates the
entry within the table. Each entry is one word in length, being a cpu
address.

The address of the 16 tables are:

    XML >0x  >0D1A through >0D39   In console ROM
    XML >1x  >12A0 through >12BF    "
    XML >2x  >2000 through >201F   Low memory expansion
    XML >3x  >3FC0 through >3FDF    "
    XML >4x  >3FE0 through >3FFF    "
    XML >5x  >4010 through >402F   Peripheral cards DSR space
    XML >6x  >4030 through >404F    "
    XML >7x  >6010 through >602F   Cartridge ROM
    XML >8x  >6030 through >604F    "
    XML >9x  >7000 through >701F    "
    XML >Ax  >8000 through >801F   Decoded as >8300, i.e. scratch-pad
    XML >Bx  >A000 through >A01F   High memory expansion
    XML >Cx  >B000 through >B01F    "
    XML >Dx  >C000 through >C01F    "
    XML >Ex  >D000 through >D01F    "
    XML >Fx  >8300 through >831F   Scratch-pad RAM

So for instance:
will get its vector from address \>B006 and enter assembly language at
whatever address is found in \>B006.

Now all you have to do is:

Place the desired assembly return address at \>B006

Place the GROM address of your `XML `instruction on the subprogram stack
(shown above as \>xxxx )

Call the desired GPL subroutine (shown above as \>6234).

When the GPL subroutine performs its `RTN `(or `RTNC`) it will return to
the XML instruction at address \>xxxx, wich will cause the GPL
interpreter to relinquish control to your assembly program, at whatever
address you placed at \>B006. Remember that the workspace will still be
\>83E0, the GPL workspace, so the first thing to do will probably be to
switch to your own workspace.

Note that XML does not affect the status byte, so you will be able to
test the Cnd bit, if the GPL subroutine returned with `RTNC`.

       MOV  @&gt;837C,R1     Get GPL status byte
       SLA  R1,3          Test Cnd bit (value &gt;20)
       JOC  SET
       JNC  NOTSET 

###  Returning to assembly: the hard way

If you don't have a GRAM card, things are getting though: how are you
going to re-enter assembly? It should be possible since GPL is entered
upon power-up and you are executing an assembly program: obviously there
is a way in! However it may not be possible to resume assembly at an
arbitrarily chosen location...

One way around this problem is to search the console GROMs for sequences
of bytes that resemble an `XML `instrustion, i.e. \>0F \>xx, where \>xx
is a suitable table location.

One such sequence is found at GROM address \>36B8 : it contains \>0F
\>4B, which could be interpreted as an XML to a vector found at \>3FF6.
(It is actually part of the instruction `MOVE >000F,@>834A,@>834B` but
we are reading it out of sequence.) The actual location of this
instruction may vary according to the GROM version, so you'd better use
your favorite memory editor to locate the \>0F, \>4B string.

All we have to do then is to use \>36B8 as a GPL return address, and
push it on the stack as shown above (\>xxxx). Then place our assembly
return address at \>3FF6 and enter the GPL interpreter, just as above.

##  The DSWGR and RTGR mechanism

As you know, GROMs are accessed via a set of four addresses in cpu
memory:
\>9800 to read data (this is known as the GROM base)
\>9802 to read the current address
\>9C00 to write data (GRAM only)
\>9C02 to set the address.

However, provision is made to use upto 16 sets of such addresses, \>0004
bytes apart, which give access to as much as one megabyte of GROM/GRAM.
This never became a reality at Texas Intruments (in fact the console
GROMs answer to each and every base), but most GRAM cards make use of
this trick to implement more than 64K of GRAM.

Interestingly, TI provided GPL opcodes to call routines located in
another base. Unfortunately, one of them is buggy!

The `DSWGR `opcode has the following syntax:

           DSWGR base,address

Where *base* can be either a variable or a constant, whereas *address*
has to be a variable. So, in practice you could type:

       DSWRG &gt;9804,@&gt;8300        Branch at &gt;604A using GROM port &gt;9804

`DSWGR `pushes both the current base and the return address on the
subroutine stack, then changes the base value at \>83FA and takes the
branch.

Routines called in this way simply return with:
which recovers the base and the return address from the stack.

Unfortunately, `RTGR `also performs a dummy write to GROM/GRAM memory,
at the address corresponding to the current base. That is, it erases
GRAM byte \>9800 when returning to base \>9800, GRAM byte \>9804 when
returning to \>9804, etc. Why TI did that is beyond my understanding.
Maybe they thought it didn't matter since GROMs cannot be written to? Or
because GROMs were limited to \>1800 bytes, GROM 8 would cover
\>8000-97FF so the \>98xx addresses would not be a problem? Well, it may
be a problem with GRAM cards, so be aware of it.

Anyhow, there is no `DSWGR`-callable routine in the console GROMs, and
I'm not aware of any cartridge that contains one. Which does not mean
that you cannot write your own!

###  Calling from assembly

To call a `DSWGR`-type routine from assembly, you cannot use GPLLNK. You
have to resort to the 'manual' technique explained above. Namely:

Push the return address on the subroutine stack

Push the current GROM base (that you can get from \>83FA)

Call the desired GPL subroutine.

##  The \>0010 and \>0012 calling mechanism

At address \>0010 in the console GROMs, is a routine which is the GPL
equivalent of DSRLNK. It is used to call by name DSRs and subprograms
(e.g. CALLs) that can be located either in peripheral card ROMs, or in
GROMs.

The routine expects the name of the DSR or subprogram to be placed in
VDP memory, at an address specified by word \>8356 (which points at the
size byte of the string). The \>0010 routine also FETChes one data byte
from GROM memory: if it's \>08 it will scan the DSR lists, if it's \>0A
it will scan the subprogram lists.

The routine first checks the name and returns with the Cnd bit set if
its size is zero, or larger than 7. Then it calls `XML >19` which scans
the peripheral cards DSR space at \>4000-5FFF. If the desired
subprogram/DSR is found there, the `XML `returns with the Cnd bit reset.

If the subprogram/DSR is not found in any card ROM, the routine goes on
scanning GROM headers (i.e. \>0000,\>2000, \>4000, etc upto \>E000). To
this end, it first saves the GROM base on the subprogram stack. Then it
calls `XML >1A` with performs the scanning. Note that `XML >1A` will
scan all sixteen GROM bases and change the current base in \>83FA
accordingly.

GPL subprograms and DSRs called by this mechanism must return by
branching to a subroutine found at address \>0012 in the console GROMs.
This routine simply retrieves the initial GROM base from the top of the
stack, sets it, then returns directly to the caller of the \>0010
routine.

Note that for some reason, subroutine \>0010 leaves two bytes of garbage
on the stack before it saves the current GROM base. This is why routine
\>0012 must be used: it knows that it should retrieve the base, then
decrement the pointer, then perform a `RTN`.

###  Calling from assembly

This is not as easy as it seems. We cannot call routine \>0010 directly,
because it expects a data byte from GROM, that we cannot pass from
assembly. Fortunately, the `FETC `instruction is the very first one in
the routine, so we could just skip it as follow:

Read the 2-byte instruction at GROM address \>0010: it will be a branch
the actual location of the routine.

Doctor it so as to increment the address by two (`ANDI `with \>1FFF,
then `INCT `it)

Place \>08 or \>0A in byte \>836D.

Now enter GPL at our doctored address, by performing a 'manual' GPL call
as described above.

Another way around is to do the scanning ourselves, but that's not easy
either. We can obviously use the DSRLNK routine to search peripheral
card ROMs. But assuming the subprogram/DSR is not found there, how to
search GROM headers?

We cannot call `XML >1A` directly because it does not return to its
caller, instead it re-enters the GPL interpreter. Plus, routine \>0010
contains a number of intricacies to repeatedly call `XML >1A`, that
would make our life really difficult.

So the best way is probably to write a routine that scans the GROM
headers and calls the appropriate GPL subprogram/DSR. This would be
cleaner than calling subroutine \>0010 with the dirty trick described
above.

Initial version 3/6/02 posted on OLUG
Revision 1. 1120/02 Turned into a webpage
[Back to the TI-99/4A Tech Pages](titechpages.md)
