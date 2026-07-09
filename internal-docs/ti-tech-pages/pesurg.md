# PE-box surgery

The purpose of this page is to show you how to take apart the peripheral
expansion box. There may be several reasons why you would want to do so:

Cleaning the contacts.

Switching to another wall-socket voltage (old-style PE-boxes don't have
a selector).

Fixing the power supply.

Changing the transformer's fuse (i.e. not the main fuse).

[Malfunction symptoms
](#Symptoms)[\_Finer diagnostic](#better%20diagnostic)

[Opening the PE-box
](#opening)[\_Cable connection card
](#connection%20card)[\_Disk drive
](#drive)[Cleaning the bottom connectors
](#connector%20plate)[Changing the mains voltage
](#change%20voltage)[Fixing the power supply board
](#fix%20power%20board)[Fixing the transformer](#transformer)

[Putting it back together](#putting%20it%20back)

##  Malfunction symtoms

No power at all

The main fuse may be blown. No need to take apart the PE-box: it is
accessible from the rear.

A connection to the mains switch may be broken, or the switch itself is
defective. Open the box, fix the connection or change the switch (almost
any kind of mains switch will do).
The fan doesn't work, but everything else is ok.

The fan may be dead: good occasion to replace it with a less noisy
one... Be aware that it's a high voltage (115 V) fan though, not a 12
volts fan like in PCs.

One of the fan wires may be broken: solder it back.

The fan works but nothing else does: no lights appear on any of the
cards, the disk drive doesn't spin at power up.

It is likely that the transformer is faulty (most probably, its fuse has
blown).

It may also be the power-supply board, but in this is less likely. In
this case you should still measure one or two of the three voltages
present on the PE-box bus (see [below](#better%20diagnostic)).
The card lights go on, but the disk drive does not spin / its red light
does not lit upon disk access.

A voltage regulator on the power board may be dead. There is one for +5V
and one for +12V, so if only one of these voltages is missing in the
drive power cable you can be pretty sure that the problem is there.

Alternatively, the corresponding wire might be broken.

Random malfuntion of several cards

Contacts may be dirty inside the bottom connectors.

Ditto for the cable connector, to the side of the console.

####  For a better diagnostic

Get a voltmeter and measure voltage accros the silver part of the bottom
plate and slot connections number 1 (the first at the rear), 57 and 59.
(the last two at the front). Either side of the slot is ok, since power
always comes through two opposite pins (i.e. 1+2, 57+58, 59+60).

You should get:

More than +5 volts on pin 1

Less than -12 volts on pin 57

More than +12 volts on pin 59

    2 ------------------------------ 60 Front
    1 ------------------------------ 59
      |                           ||
     +5                        -12 +12

All voltages will be lower if cards are still present inside the box
(approx +8, -16, +16V) than in a bare PE-box (+13, -25, +25V). This is
normal.

If one of the voltages is missing, there may be a problem with the power
board, or the corresponding wire may be broken.

If all are missing, the problem is likely with the transformer (in which
case the drive won't have power either).
After removing the disk drive (see [below](#drive)), check the voltage
on the "molex" connector, between any of the center connections and the
side ones. You should read +5 volts on one side and +12 volts on the
other.

     _________
    | O O O O |
      \/  \/
     +5  +12

If one of the voltages is missing, the problem is either with that wire
or with the corresponding regulator on the power board.

If both are missing, the problem may be with the transformer (in which
case the PE-box slots won't have power either) or both regulators/wires
are defective. E.g. if you yanked on the power cord when pulling out the
drive...

##  Taking apart the PE-box

First of all, unplug the power cord and store it away in a locked
drawer. We're talking at least 100 volts upstream of the mains switch
and this is not something to take lightly: it could easily kill you. So
take no risk and **make sure the PE-box is not connected to the wall
power until it's fully reassembled**. You have been warned. If you kill
yourself, don't come and complain to me!

####  Remove/clean the connection card

To remove the cable connection card, you will need to unfastened the
brace that fixes it to the back of the PE-box (one screw).

If you want to clean this card, remove the two handles (pull them apart)
and the four screws that lock the metal case. The label can stay and
will serve as a hinge. Take the card out and clean the connections at
the place they fit into the slot at the bottom of the PE-box. A
toothbrush dipped in baking soda (sodium bicarbonate) will do. You might
also use a "scrotch-brite" scratchy sponge, or even very fine sandpaper.
In any case, make sure not to touch the contacts with bare fingers after
you are done, as skin oils will promote oxidation.

If you determine that there is a problem with this card, or with the
'firehose' connection cable, refer to my [PE-Box
page](pebox.htm#Fixing%20firehose) for instructions on how to fix it.

####  Remove the drive, then its card

To remove the disk controller card, it is best to first pull out the
drive. There are two screws, in slotted holes, under the PE-box (cyan
arrows in [this picture](pesurg3.jpg)), and two more on top of the drive
under the PE-box lid (see [this picture](pesurg1.jpg)). Remove them and
gently pull out the drive, without putting too much strain on the
cables.

Now reach inside the hole and disconnect the flat cable from the card
connector, at the very bottom of the drive compartment. Here's a
[picture](pesurg2.jpg). Then disconnect the drive power cord and store
the drive in a safe place.

Now you can remove the controller card and clean it as described above.

####  Open the box

Grab your screwdriver and start removing the screws that hold the box
closed. There are plenty: 7 at the bottom ([picture](pesurg3.jpg)), 6 at
the back, and one on each side ([picture](pesurg4.jpg)).

Pull out the mains push-button from the front panel (if so equiped).
Then gently slide out (foward) the front-and-side part of the box from
the bottom-and-back. See [this picture](pesurg5.jpg).

###  To clean/fix the connector plate

Remove the screws that hold the 8-slot bottom plate in place
([picture](pesurg6.jpg)). Together with it will comes the metal guide
that insures each card slides properly in place.

The connector plate is a very simplistic PCB: it just connects every
line between eight 60-pin card edge connectors. Four power supply lines
are attached to it, that go to the power supply board. This is
unregulated power, to be used by voltage regulators installed on cards.
See this [picture](pesurg11.jpg).

You can clean the connectors with an old toothbrush diped in a solution
of baking soda (sodium bicarbonate). Rince with distilled water, shake
the water out and dry thouroughly (e.g. overnight) before attempting to
use.

If you suspect one trace is broken, locate it and bridge the gap with
solder.

Now is a good time to dust off the rest of the box, particularly the
fan, and the transparent plastic things that lead the light from the
card LEDs to the front panel...

###  To change the wall-power voltage

The "new" PE-boxes, those with a rocking mains switch, have a rotatory
selector that lets you pick up the desired voltage. The older models,
those with a mains push-button, don't have such a selector even though
the transformer can often accept the same range of voltage.

If you look at the transformer, you will see that its primary (at the
base of the transformer) has five power wires, plus two for the fan. One
is common to all voltages (white, soldered to the mains switch), one is
connected to the mains switch (red for US power), the other three are
parked on an isulating pad (brown, orange, and black). See this
[picture](pesurg12.jpg).

To use your PE-box with another voltage, simply exchange the red wire
for the one corresponding to your local wall-power voltage.
Unfortunately, I don't know for sure which color corresponds to which
voltage (if you figure it out, let me know). From the order the wires
come out of the transformer, and by measuring the AC voltage on the
three spare wires when the tranfromer is powered by the red one, I was
able to make the following guess: Black = 100 VAC, Red = 115 VAC, Brown
= 220 VAC, Orange = 240 VAC. But remember this is only a guess...

###  To clean/fix the power supply board

The power-supply board is mounted on a plastic brace that holds it
vertical, along the left side of the PE-box. To remove this brace,
unfasten the two screws that fix it to the base of the box (see [this
picture](pesurg7.jpg): follow the screwdrivers), then slide it off
towards the left side.

Disconnect the three sets of wires. All connectors are built on the same
model, with self-locking clamps that fit into holes in the board (see
[picture](pesurg8.jpg)). To remove them, flip the board over, spread the
two clamps to release them, and push them into their holes. The plug
should now be tilted up (see [picture](pesurg9.jpg)), and it's a simple
matter to pull it out. To put it back, install it at the same angle,
them push the clamps down into the holes.

It's easier to work on the board if you remove the plastic brace, not to
mention that there are two components under the board, hidden by the
brace (purple arrows in [this picture](pesurg10.jpg)). So remove the 4
screws that hold it in place. As the board is perfectly square, make
sure to mark which side goes where on the brace (if you forgot, refer to
the above picture).

####  To change the voltage regulators for the drive

The +5 volts regulator is the flat 3-legs thing labeled 7805. The +12
volts regulator is the big silver eye-like thing in the middle, they are
labelled in cyan in the above picture.

Desolder and replace them with a similar part. Or possibly with one that
gives out more current so you can power both a hard-drive and a floppy
(I haven't tried that).

###  To change/fix the transformer

The transformer is bolted to the bottom of the box with 4 bolts that are
welded in place (two of which are visible in [this
picture](pesurg12.jpg)). Use a wrench (or pliers) to remove the 4 nuts,
then gently lift up the transformer. It will remain attached to the
mains switch by at least the white wire, but that's ok. Here's a
[picture](pesurg13.jpg) with the transformer removed, in which you can
see the 4 bolts protruding from the bottom of the PE-box.

####  To change the fuse

The fuse is hidden inside the transformer's white plastic case. As it is
hopelessly glued together with resin, the best way to access this fuse
is to break a hole inside the plastic, at the bottom of the transformer,
opposite the wires. See [this picture](pesurg14.jpg).

There is no fuse holder, as a matter of fact it may be that the fuse is
just a thicker portion of wire. Thus, if you want to install a new fuse,
you will have to solder one in place. In this respect, buying fuses that
have axial leads will help.

I don't know for sure what's the rating of this fuse. The writing was
hard to read on mine, and all I could decipher was 12A / 250V. Which
seems like an awfull lot (3 kilowatts???). So maybe it's 1.2A ? In
doubt, I put in a 7A fuse and it worked. Of course, maybe the
transformer is gonna fry next time I accidentally short +12V with -12V,
as I've been known to do...

####  To replace the transformer

To install a new transformer, you will need to desolder the white wire
from the mains switch, then solder back the new transformer. Make sure
you insulate your work with electrical tape, real well: we don't want to
shortcut these two terminals!

Then connect all the wires, including the "unused" ones that should be
parked on the dedicated terminal.

##  Putting it back together

Screw the power board back on its plastic brace.

Reconnect the power board with the transformer, the drive's power lines
and the bottom plate's power lines.

Slide the plastic brace in place, and fasten the two screws at the
bottom.

Reinstall the bottom plate (the one with the 8 connector slots) with the
metal guides on top of it.

Slide the bottom+back part of the box into the front+side part. Make
sure not to trap any wire. Put all the screws back (except the ones for
the drive: those that have slotted holes). Push the mains push-button
in.

Go get the power cord and plug it in. Turn power on and check if all the
required voltages are present where expected (see
[above](#better%20diagnostic)). If so, turn power off and proceed.

Put in the floppy-disk controller card.

Reconnect the drive flat cable and power supply.

Put the drive back into its hole and fasten the four screws (two on top,
two at the bottom).

Put in the cable connection card. You may dispense with fastening it in.

Test your system. If it works fine, install the remaining cards and
enjoy. If not, start over... or buy another PE-box.

Preliminary 7/3/02 Not for release

Revision 1 8/27/02 Added pictures. OK to release

 

[Back to the TI-99/4A Tech Pages](titechpages.md)
