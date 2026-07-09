# Floating point numbers arithmetic

[Radix 100 notations
](#Radix%20100)[RAM addresses
](#addresses)[Floating point routines](#routines)

## Radix 100 notation

In the TI-99/4A real numbers are represented in radix 100, floating
point notation. This notation uses 8 bytes per number. It is different
from that used on PCs and yet extremely efficient: at equal processing
power a TI-99/4A would do much better than a PC.

#### Exponent

The exponent is 1 byte long. Its value is biased by \>40:

\>40 is E+0
\>41 is E+1
\>3F is E-1
etc.

#### Mantissa

The mantissa is seven bytes long. Each byte contains a number between 0
and 99 (\>00 and \>63), providing up to 14 significant digits. The
decimal point is assumed to be after the first mantissa byte (i.e. after
the second digit).

#### Negative number

Both the exponent and the first mantissa byte are negated, i.e. the
first word is negated.

#### Zero

The number zero is an exception: the exponent and the first mantissa
byte are both zero (i.e. the first word is \>0000). The remaining bytes
can have any value.

#### Examples

    Exponent Mantissa                      Value

    >40      >01 >02 >03 >04 >05 >06 >07   1.020304050607

    >41      >01 >02 >03 >04 >05 >06 >0B   10.20304050611

    >3F      >05 >00 >00 >00 >00 >00 >00   0.5

    >BF(-41) >FF >02 >03 >05 >05 >06 >12   -10.20304050618

    >00      >00 >xx >xx >xx >xx >xx >xx   0 (any exponent!)

##  Scratch-pad addresses

Most ROM and GROM routines that deal with real numbers use the following
addresses in the scratch-pad RAM.

**FAC**: an 8 bytes area, from \>834A to \>8351 (Floating point
ACcumulator)
**ARG**: an 8 bytes area, from \>835C to \>8363 (ARGument accumulator)
**SGN**: byte \>8375, sign of FAC (00 = positive, \>FF = negative)
**EE**: byte \>8376, copy of FAC exponent
**ERCODE**: byte \>8354, error code returned by floating point
calculation routines.
**VSPTR**: bytes \>836E-\>836F, pointer to a data stack in VDP memory.
In Basic it is called the value stack, but you can use it even if your
program does not run with Basic: just put a relevant value in \>836E-F

#### Error codes

The following error codes can be found at \>8354, after a real number
operation. In general, the Cnd bit of the GPL status byte (\>837C) is
set, if an error occured.

\>01 Overflow (warning) / Division by 0 (error)
\>02 Syntax error
\>03 Integer overflow, on conversion
\>04 Square root of a negative number
\>05 Negative number raised to non-integer power
\>06 Log(0) or log of a negative number
\>07 Trigonometric function error

##  Floating point calculations routines

A number of assembly and GPL routines that perform advanced calculations
with floating points are at your disposal. In general, they should be
called from GPL: the assembly routines as XML, the GPL routines with
CALL. However, you can also use them in assembly, with a suitable
[calling routine](headers.htm#GPLLNK).

You'll see in the table below that many floating point operations are
performed by two distinct routines: one uses two numbers in the
scratch-pad (in FAC and in ARG), the other a number in FAC and a number
in the VDP stack. The stack pointer (in \>836E-F) is automatically
decremented by 8 to point at the next number once the operation is
completed.

#### XML routines.

|  |  |  |
|----|----|----|
| Mnemonic | XML | Use |
| ROUND1 | [\>01](gpl2.htm#xml01) | Rounds number in FAC from 9 to 8 bytes. |
| ROUND | [\>02](gpl2.htm#xml02) | Rounds this number. Number of decimals in \>835C. |
| STST | [\>03](gpl2.htm#xml03) | Test number in FAC and saves status. |
| OVEXP | [\>04](gpl2.htm#xml04) | Overflow / underflow. |
| OV | [\>05](gpl2.htm#xml05) | Overflow (part of the above procedure). |
| FADD | [\>06](gpl2.htm#xml06) | Adds two real numbers: FAC = FAC + ARG |
| FSUB | [\>07](gpl2.htm#xml07) | Substracts a real from another: FAC = ARG - FAC |
| FMUL | [\>08](gpl2.htm#xml08) | Multiplies two reals: FAC = FAC \* ARG |
| FDIV | [\>09](gpl2.htm#xml09) | Divides real by another: FAC = FAC / ARG |
| FCOMP | [\>0A](gpl2.htm#xml0a) | Compares two reals (FAC with ARG) |
| SADD | [\>0B](gpl2.htm#xml0b) | Addition with stack: FAC = FAC + real_on_stack |
| SSUB | [\>0C](gpl2.htm#xml0c) | Substraction with stack: FAC = real_on_stack - FAC |
| SMUL | [\>0D](gpl2.htm#xml0d) | Multiplication with stack: FAC = FAC \* real_on_stack |
| SDIV | [\>0E](gpl2.htm#xml0e) | Division with stack: FAC = FAC / real_on_stack |
| SCOMP | [\>0F](gpl2.htm#xml0f) | Comparison with stack: FAC with real_on_stack |
| CSN | [\>10](gpl2.htm#xml10) | Converts a string (ptr in \>8356) to a real (in FAC), from Basic |
| CSNGR | [\>11](gpl2.htm#xml11) | Same as above, entry point from GPL. |
| CFI | [\>12](gpl2.htm#xml12) | Converts real to integer (from FAC to FAC) |
| VPUSH | [\>17](gpl2.htm#xml17) | Pushes a value on the VDP stack, from FAC |
| VPOP | [\>18](gpl2.htm#xml18) | Pops a value from the VDP stack to FAC |

#### GROM routines

|  |  |  |
|----|----|----|
| Mnemonic | Address | Use |
| CNS | [\>0014](gpl2.htm#g@0014) | Convert number to string (from FAC to FAC, infos in \>8355-57) |
| INT | [\>0022](gpl2.htm#g@0022) | Convert real to integer (from FAC to FAC) |
| PWR | [\>0024](gpl2.htm#g@0024) | Power-of-ten routine: FAC=FAC \* 10^ARG |
| SQR | [\>0026](gpl2.htm#g@0026) | Square root routine: FAC=SQR(FAC) |
| EXP | [\>0028](gpl2.htm#g@0028) | Exponentiation routine: FAC=e^FAC |
| LOG | [\>002A](gpl2.htm#g@002a) | Logarithm calculation: FAC=ln(FAC) |
| COS | [\>002C](gpl2.htm#g@002C) | Cosine calculation: FAC=cos(FAC) |
| SIN | [\>002E](gpl2.htm#g@002e) | Sine calculation: FAC=sin(FAC) |
| TAN | [\>0030](gpl2.htm#g@0030) | Tangent calculation: FAC=tan(FAC) |
| ATN | [\>0032](gpl2.htm#g@0032) | Arctangent calculation: FAC=atn(FAC) |
Revision 1. 6/23/99 OK to release
[Back to the TI-99/4A Tech Pages](titechpages.md)
