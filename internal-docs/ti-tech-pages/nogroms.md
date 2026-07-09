# Disabling the GROMs

Some Gram cards, like the german 128K GramKarte or the HSGPL card, have
the ability to emulate the console GROMs. Because the data bus drivers
for the side connector are stronger than real GROMs, the card will
allways override console or cartridge GROMs. However, there is a risk
that this will end up damaging the GROMs (although it never happened to
me).

The obvious solution is to remove the console GROMs. (Note that this
will only work with the HSGPL card, since it's the only one with GROM
address readback.With the German GramKarte you must leave at least one
GROM in the console to keep track of the current address.)

However, removing the GROMs has several drawbacks:.

The console becomes unbootable without a PE-box with a HSGPL card in it.

If anything gets corrupted in the card EEPROM memory, the console may
not boot.

To overcome these problems, I suggest a very simple solution: install a
switch that will disable console and cartrige GROMs so you can use your
HSGPL card instead. By just toggling the switch you can go back to the
original GROMs.

[How does it work
](#How)[Installing the switch
](#Doit)[Testing](#Testing)

###  How does it work?

The chip that generate the GROM selection signal is a 74LS03
open-collector NAND gate. Its active-high output is then inverted by a
74LS04 to provide the active-low GS\* selection signal. It also gates
the GREADY line and prevents it from stalling the CPU when the GROMs are
not addressed (GROMs set GREADY as "not ready" by default).

+-----+        ,--WWW---+5V      4.7K
|  Y7*|--,     |              ,--WWW---+5V
|  Y6*|--=|)o--+---+----,     |
|     | 74LS03     |   ,=|)o--+------Hold*
|     |            |   |
|     |            |   &#39;--o&lt;|---GREADY
|     |            |
|     |            &#39;--|&gt;o-------GS*
+-----+             74LS04

Since the 74LS03 has open-collector outputs, it needs a pull-up resistor
to +5V. We are going to install an SPDT (single-pole dual-terminal)
switch to toggle this resistor between pull-up and pull-down. In the
latter position, GROMs cannot be selected any more!

###  Installing the switch

Open your TI-44/4A console, refer to my [console surgery
page](surgery.md) for detailed intructions.

Locate R503, the resistor we want. It's a 1 kiloOhm resistor, so the
color code is: brown, black, red. You'll find two such resistors in the
back right corner of the motherboard. One is outside the metal case, we
don't want it. The next resistor inwards is the one we want:

     ,-,-  ,-,-    ===|
     | |   | |     ===| Side connector
     | |   | |     ===|
     | |   | |     ===|
    +'-'  +'-'    |    notch
     ,-,-  ,-,-   '-----,
     | |   | |   , ,<-, | <- Cut this end
     | |   | | , R R  R | attach wire to it
     | |   | | R ' '  ' |
    +'-'  +'-' '        | Get +5V from any +
            --R--       | ground from any -
                        |
    ---------------,    |
                   |   O|
                   |O|

Cut the end of the resistor that's away from the board edge, close to
the notch near the side port. Leave the stump of the resistor in the
board, as it must bring current from one side of the board to the
other.

Solder a length of wire to the free end of the resistor. Carefully
isolate the connection with electrical tape, so it won't make contact
with anything on the board. Here is a [picture](nogroms.jpg).

The other end of the wire should go the the middle connection of the
switch (the pole).

Connect one switch terminal to +5V, the other to ground. You will find
+5V at the upper right pin of any 74LSxxx chip, and ground almost
everywhere, for instance at the bottom left pin of any 74LSxxx chip
("up" is where the notch is on a 74LS chip).

                        ,--------Resistor
    Ground ----------o 0_o---------- +5V

When the switch is on the left, GROMs are disabled. When it's on the
right, GROMs work as usual.

Drill a hole in the plastic casing of the console and mount the switch
in it (see [picture](switches.jpg)).

Close your console.

###  Testing

The easiest way to ensure that the switch works is with my RIP debugger.
You can try other debuggers, but many won't work because they call the
keyboard scanning routine in the console ROMs and this routine gets the
ascii codes from the console GROMs. RIP does its own keyboard scanning
and is thus independent of the console GROMs and ROMs. It can be
downloaded from [here](download.htm#rip).

To load and run RIP from Extended Basic, type:

CALL INIT
CALL LOAD("DSK1.RIP/O")
CALL LINK("RIP")

From Editor/Assembler, use option 3 to load RIP/O, then run program
"RIP".

Press Fctn-8 three times to select G-memory. Enter address 0 and press
enter. You should see the contents of the first console GROM: \>AA 02 00
00 etc.

Now toggle the switch and press Fctn-4 and Fctn-6 to move up and down
memory. You should only see zeros now.

Toggle the switch back, press Fctn-4 or Fctn-6 and watch the GROM
reappear.

Press Fctn-8 and enter address 6000, the address of the cartridge that
you have plugged in. Toggle the switch and verify that the cartridge
GROMs are equally disabled (NB cartridge ROMs are not!).

Toggle the switch back to the position where the console GROMs are
enabled. Press and hold Fctn= for a short while to exit RIP.
Revision 1. 4/27/02  Preliminary.
Revision 2. 4/29/02 OK to release.

[Back to the TI-99/4A Tech Pages](titechpages.md)
