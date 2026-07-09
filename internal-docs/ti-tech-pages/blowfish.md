# Blowfish encryption

This page discusses an assembly language implementation of the
encryption algorithm known as Blowfish.

[Introduction
](#intro)[Bitwise XOR and modulo additions
](#xor%20and%20modulo%20add)[Feistel network
](#Feistel%20network)[The function F
](#the%20function%20F)[Encoding and decoding routines
](#Encode%20and%20decode)[Key generation
](#key%20generation)[Testing
](#Test)[References](#Refs)

###  Introduction

Blowfish is a fast and secure encryption algorithm, designed by B.
Schneier in 1993. Since that time it has never been cracked, in spite of
numerous attempts. It is designed for speed, by using only simple
operations like additions and bitwise exclusive or (XOR). Its speed and
strength are due to the fact that it uses a huge key, over 4 Kbytes of
random numbers! Obviously, it is impossible to memorize such a key, but
the Blowfish designer provided an elegant solution: a unique key is
selected once and for all, and subsequently altered by a password of
your choice. The alteration is performed via repeted rounds of Blowfish
encryption, in other words the key encodes itself. This self-encoding
process is a bit lengthy but only needs to be performed once per
session.

###   Bitwise XOR and modulo additions

Before we go further, let me discuss briefly the two mathematical
operations that are at the heart of Blowfish: modulo additions and
bitwise XOR.

**Modulo additions** simply means that there is a maximum value that
cannot be trespassed. If the result of the addition is greater than this
value, it "wraps aroud" and restart from zero. For instance, additions
modulo 216 are 16-bits additions: \>F000 + \>2002 should be
\>11002, but because only 16 bits are allowed, the result is \>1002
instead. Blowfish uses modulo 232 additions, which means the
greatest possible value is \>FFFFFFFF.

**A bitwise exclusive OR** compares two numbers bit-by-bit: when the two
bits are identical the corresponding bit in the result os 0, when the
bits are different the result is 1. In other words, the truth table is:
0 xor 0 = 0, 0 xor 1 = 1, 1 xor 0 = 1, 1 xor 1 = 0.

XOR is a commonly used encryption tool, as it very effectively mangles
the original value (aka plaintext):

        >00100110 (plaintext: >46 or 'F')
    xor >10110010 (key: >B2)
        >11010100 (ciphertext: >D4)

XOR is commutative, so the plaintext can be retrieved by just XORing the
ciphertext with the key again:

        >11010100 (ciphertext: >D4)
    xor >10110010 (key: >B2)
        >00100110 (plaintext: >46)

Unfortunately it is also associative, so if an adversary manages to
guess one word in the plaintext, he can obtain the key by combining
plantext and ciphertext:

        >00100110 (plaintext: >46)
    xor >11010100 (ciphertext: >D4)
        >10110010 (key: >B2)

Several strategies are possible to overcome this problem, by combining
XORs in more complex functions.

###  Feistel network

Blowfish is is a block cipher, which means that is operates on blocks of
a fixed size (64 bits, or 8 bytes) that are encoded independently from
each other. It is based on a Feistel network, an algorithm that operates
as follows:

Given a 64-bit value X, split it in two 32-bit parts: Xl (leftmost 4
bytes) and Xr (rightmost 4 bytes).

Do Xr = Xr xor F(Xl)

Do Xl = Xl xor F(Xr)

The function F will be discussed later, for the moment being it is
sufficient to know that it returns an arbitrary number that depends on
the value of its argument.

It is easy to see that the above algorithm is symetrical and can be
taken backwards to decipher the ciphertext. To make the code harder to
brake, the process is repeated 8 times. In addition, at each step Xl and
Xr are XORed with arbitrarily chosen random numbers, that change at each
pass. These numbers are stored in an array called P that contains
eighteen 32-bit numbers and forms one part of the key (there are 18
values instead of 16 because an extra XOR is performed after the last
step).

The algorithm thus become:

- Split the plain 64-bit value X in two 32-bit halves: Xl and Xr
- For i = 1 to 16 do
  - Xl = Xl xor P\[i\]
  - Xr = Xr or F(Xl)
  - Swap Xl and Xr
- Undo the last swap
- Xr = Xr xor P\[17\]
- Xl = Xl xor P\[18\]
- Recombine Xl and Xr into a ciphered 64-bit value
To decode the final value back to the original one, the same method is
used, except that the array P is walked backwards:

- Split the ciphered 64-bit value X in two 32-bit halves: Xl and Xr
- For i =18 to 3 step -1 do
  - Xl = Xl xor P\[i\]
  - Xr = Xr or F(Xl)
  - Swap Xl and Xr
- Undo the last swap
- Xr = Xr xor P\[2\]
- Xl = Xl xor P\[1\]
- Recombine Xl and Xr into a plain 64-bit value

Elegant, isn't it?

###  The function F

Before we get to this mysterious function F, let me introduce two
important cryptographic concepts: one-way functions and non-linearity.

A **one way function** is a function where it is very easy to calculate
F(x) given x, but very difficult, if at all possible, to work backwards
and get x from F(x). A naive exemple would be: y = x2 It is
easy to square a number, but much harder to extract its square root. Of
course, real-world one-way functions are more complex than this, but you
get the idea.

**Non-linearity** means that even a tiny change in the key results in
major changes in the ciphertext. It is important so that an adversary
does not get the feeling that "it begins to make sense" as he guesses
more and more letters in your password. Ideally, even if he has all
characters correct but for a single bit, the ciphertext should still be
just as mystifying as if he had it all wrong.

A good way to achieve non-linearity is to use S boxes. A S box is
nothing else than an array of randomly chosen numbers. A non-linear
function may just return S\[x\], i.e. the element at position x in the
array. Since these elements are chosen at random, chances are that two
neighbouring elements have vastly different values. In addition, this is
also a one-way function since a given number may appear at several
places in the array: given this value it is impossible to tell which
element was the one that returned it.

There is one problem with this approach though: remember that the
function F in Blowfish takes a 32-bit number as an argument and returns
another 32-bit number. If the function was to perform a simple array
lookup we would need a S box containing 4 billions 32-bit numbers! To
overcome this problem, the function F in Blowfish breaks up the 32-bit
input parameter in 4 bytes. Each byte is used to fetch a 32-bit number
from a different S box (S1 through S4), and the four numbers are
combined with additions and XOR operations. Blowfish uses four S boxes
to avoid the symetry problems that could occur for numbers like
\>11112222 if only one box were used. Together with the P box, the four
S boxes make up the encryption key.

The algorithm for function F is:

Given a 32-bit value X, break it in 4 bytes: a, b, c and d.

Return (( S1\[a\] + S2\[b\] ) xor S3\[c\] ) + S4\[d\]

Note how fast and easy it is to obtain F(x) from x: only 4 lookups, two
additions (modulo 232) and an exclusive or. By contrast,
given F(x) it is very hard to figure out what the value of x was. It may
even be impossible, as F may return the same number for different values
of x. Thus F is a non-linear, one-way function.

###  Encoding and decoding routines

With all this theory behind us, it is now time for some assembly code.
Here is my implementation of the coding and decoding functions for
Blowfish. Note that they have been optimised for speed: they almost
exclusively use registers and my advise would be to use a workspace
located in the fast scratch-pad memory, such as \>8300. For the same
reason, I did not implement F as a distinct function, but rather
integrated it into each routine. This saves us the 16 calls and return
operations. To further speed up these routines, you may "open the loop",
viz copy F 16 times in a row instead of using a counter. This way, you
don't even need to swap Xr and Xl, and you save the time needed to
decrement a counter, test it, and jump back to the beginning of the
loop.

#### Encrypting 64 bits

* Encryption routine. R1=input ptr (plain), R2=output ptr (ciphered)
*---------------------------------------------------------------------
ENCOD8 MOV  *1+,5             get xL
       MOV  *1+,6
       MOV  *1+,7             get xR
       MOV  *1+,8
        CLR  3                 iteration counter i
LP1    XOR  @P(3),5           xL xor P[i]
       XOR  @P+2(3),6
        MOV  5,4               function F(x)
       SRL  4,8               -------------
       SLA  4,2               make 1st byte a pointer
       MOV  @S1(4),9          into an array of 32-bit dwords
       MOV  @S1+2(4),10       get 32-bit dword
        MOV  5,4               second byte b
       SLA  4,8
       SRL  4,6               pointer into a second array
       A    @S2+2(4),10       32-bit addition
       JNC  SK1
       INC  9                 carry to leftmost word
SK1    A    @S2(4),9
        MOV  6,4               third byte c
       SRL  4,8
       SLA  4,2               pointer into a third array
       XOR  @S3(4),9
       XOR  @S3+2(4),10       xor with previous result
        MOV  6,4               fourth byte d
       SLA  4,8
       SRL  4,6               pointer into a fourth array
       A    @S4+2(4),10       32-bit addition
       JNC  SK2
       INC  9                 carry to leftmost word
SK2    A    @S4(4),9          F(abcd) = S1[a] + S2[b] xor S3[c] + S4[d]
        XOR  7,9
       XOR  8,10              xR xor F(xL) = new xR
        MOV  5,7               swap xL with xR
       MOV  6,8
       MOV  9,5
       MOV  10,6
        C    *3+,*3+           4 bytes per entry
       C    3,@PASS
       JL   LP1               repeat 16 times
        XOR  @P+64,5           xR xor P17
       XOR  @P+66,6
       XOR  @P+68,7           xL xor P18
       XOR  @P+70,8
        MOV  7,*2+             encoded xL
       MOV  8,*2+
       MOV  5,*2+             encoded xR
       MOV  6,*2+
       B    *11
*
PASS   DATA 16*4              number of passes

#### Decrypting 64 bits

* Encryption routine. R2=input ptr (ciphered), R1=output ptr (plain)
*---------------------------------------------------------------------
DECOD8 MOV  *2+,5             get xL
       MOV  *2+,6
       MOV  *2+,7             get xR
       MOV  *2+,8
        MOV  @PASS,3           iteration counter i
LP3    XOR  @P+4(3),5         xL xor Pi
       XOR  @P+6(3),6
        MOV  5,4               function F(x)
       SRL  4,8               -------------
       SLA  4,2               make 1st byte a pointer
       MOV  @S1(4),9          into an array of 32-bit dwords
       MOV  @S1+2(4),10       get 32-bit dword
        MOV  5,4               second byte b
       SLA  4,8
       SRL  4,6               ptr into a second array
       A    @S2+2(4),10       32-bit addition
       JNC  SK3
       INC  9                 carry to leftmost word
SK3    A    @S2(4),9
        MOV  6,4               third byte c
       SRL  4,8
       SLA  4,2               ptr into a third array
       XOR  @S3(4),9
       XOR  @S3+2(4),10       xor with previous result
        MOV  6,4               fourth byte d
       SLA  4,8
       SRL  4,6               ptr into a fourth array
       A    @S4+2(4),10       32-bit addition
       JNC  SK4
       INC  9                 carry to leftmost word
SK4    A    @S4(4),9          F(abcd) = S1[a] + S2[b] xor S3[c] + S4[d]
        XOR  7,9
       XOR  8,10              xR xor F(xL) = new xR
        MOV  5,7               swap xL with xR
       MOV  6,8
       MOV  9,5
       MOV  10,6
        AI   3,-4              4 bytes per entry
       JGT  LP3               repeat 16 times
        XOR  @P+4,5            xR xor P1
       XOR  @P+6,6
       XOR  @P+0,7            xL xor P0
       XOR  @P+2,8
        MOV  7,*1+             encoded xL
       MOV  8,*1+
       MOV  5,*1+             encoded xR
       MOV  6,*1+
       B    *11

###  Key generation

If you followed the theoretical part above, you will have noted that
Blowfish uses a really large key: a P box containing 18 32-bit numbers
and four S boxes, each with 256 entries. All this adds up to 4168 bytes.
What we need now is a way to generate these boxes from a simple, easy to
remember, password.

To do this, we'll first start with an arbitrary set of random values in
the boxes. By convention these are the decimals of Pi, but any suite of
random numbers will do. The nice thing with Pi is that, if you loose the
source code, you can always reconstitute it. Also, you can verify that
the numbers provided are indeed the decimals of Pi and not values
carefully selected by a devious programmer so as to leave a weakness in
your encryption scheme, a backdoor that will allow him (or the
government) to easily decrypt all your messages...

Second, we'll XOR your password over the bytes in the P box. Your
password should be at most 56 characters (I don't know why it cannot be
longer). If it's shorter it will be repeated over and over again, until
all the bytes in the P box have been XORed with a letter from your
password.

Third, we'll Blowfish-encode an arbitrary chosen 64-bit value, using the
P and S boxes that we just created. By convention, the value chosen is
\>0000000000000000. The ciphered result will replace the first 4 bytes
in the P box. This ciphered value is then Blowfished again, using the
new version of the key, and the result replaces the next 4 bytes in the
P box.

The process is repeated 521 times, so as to replace each entry in the P
box, then in the four S boxes. Each time, the new ciphered value is
encoded again, using the version of the key that it has just created.
The whole process is quite lengthy, and takes about 3 seconds with the
routine below.

* Key generation
*---------------------------------------------------------------------
INIKEY LWPI &gt;8300             use workspace in PAD for speed
        LI   2,P               first step: fill boxes with random digits
       LI   1,PI              ----------  here: the decimals of PI
LP7    MOV  *1+,*2+
       CI   2,EOS4
       JL   LP7
        LI   2,P               second step: xor password with P box
       LI   1,KEY             -----------
       MOVB *1+,0             size of password (could truncate to 56)
       SRL  0,8               make it a word
       JEQ  WIPE              0: erase current key
 LP5    MOVB *1+,3             get 1 char
       SWPB 3
       DEC  0                 end of password?
       JNE  SK6
       LI   1,KEY             yes: restart from beginning
       MOVB *1+,0             get size
       SRL  0,8
SK6    MOVB *1+,3             get another char
       SWPB 3
       DEC  0                 end of password?
       JNE  SK7
       LI   1,KEY             yes: restart from beginning
       MOVB *1+,0
       SRL  0,8
SK7    XOR  *2,3              xor password with P box
       MOV  3,*2+
       CI   2,S1
       JNE  LP5               next word in P box
        LI   1,NULL            third step:  self-encode P + S boxes
       LI   2,P               ----------
LP6    BL   @ENCOD8           encode it, put new value in box
       MOV  2,1
       AI   1,-8              re-encode the new value
       CI   2,EOS4            until all boxes are full
       JL   LP6
       JMP  RETURN            done
*
*---------------------------------------------------------------------
* Wipe out current key
*---------------------------------------------------------------------
WIPE   LI   1,P
LP8    CLR  *1+               clear all boxes
       CI   1,EOS4
       JL   LP8
 RETURN LWPI &gt;20BA             back to caller workspace
       B    *11
*
*---------------------------------------------------------------------
* Data area
*---------------------------------------------------------------------
NULL   DATA 0,0,0,0           null string
 KEY    BYTE 11                password length
       TEXT &#39;My password&#39;     password
       BSS  45                max 56 characters
       COPY &quot;DSK4.PI/S&quot;       this file contains the digits of PI
 P      BSS  72                room for custom-made key
S1     BSS  1024
S2     BSS  1024
S3     BSS  1024
S4     BSS  1024
EOS4

Note that I also wrote a tiny routine to wipe the key out of memory,
just for security purposes.

Oh, and [here](pi_s.txt) are the decimals of Pi (in hexadecimal).

###  Testing

Finally, here are some test routines. They encode 512 bytes in an
arbitrary chosen buffer.

 TEST   EQU  &gt;D000
ENDOT  EQU  &gt;D200`
*---------------------------------------------------------------------
* Encode 512 bytes into a test buffer
*---------------------------------------------------------------------
ENCODE LWPI &gt;8300             use fast workspace
       LI   1,TEST            ptr to test buffer
LP2    MOV  1,2               overwrite it with ciphered version
       BL   @ENCOD8           encode 8 bytes
       CI   1,ENDOT           are we done?
       JL   LP2               not yet
       JMP  RETRN
*
*---------------------------------------------------------------------
* Decode 512 bytes from a test buffer
*---------------------------------------------------------------------
DECODE LWPI &gt;8300             use fast workspace
       LI   2,TEST            ptr to test buffer
LP4    MOV  2,1               overwrite with deciphered version
       BL   @DECOD8           decode 8 bytes
       CI   2,ENDOT           are we done?
       JL   LP4               not yet
RETRN  LWPI &gt;20BA             back to caller workspace
       B    *11
*
       END

To test:

Load the program.

Run INIKEY, then ENCODE.

Exit the program and make sure the content of the buffer is now
scrambled.

Load the program again, run INIKEY, then DECODE.

Check if the content of the buffer is back to what it was.
### References

B. Schneider. *Description of a New Variable-Length Key, 64-Bit Block
Cipher (Blowfish)*. Fast Software Encryption, Cambridge Security
Workshop Proceedings (December 1993). Springer Verlag, 1994, pp.
191-204.

B Schneider's company website: 

A Blowfish implementation in C language by Paul Kosher (pck@netcom.com),
found at the above website.

Revision 1. 6/14/00. Ok to release.
[Back to the TI-99/4A Tech Pages](titechpages.md)
