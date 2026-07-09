data-align="middle" />](http://nouspikel.group.sheffield.ac.uk/ti99/titechpages.md) Tech Pages
These pages are dedicated to the Texas Instruments TI-99/4A home
computer. They contain mainly technical information on the various
processors and memory chips found in the console, on peripheral cards
and cartridges and on additional hardware. Although the emphasis is on
hardware, I included numerous examples of assembly language programs to
demonstrate how to access hardware or illustrate undocumented features.

These pages are not for beginners! To follow the discussion you'll need
some knowledge of digital electronics and notions of assembly language
programming. I tried to adopt a fairly didactic style, but this is by no
means a tutorial. Nevertherless, you'll find an assembly language
primer, an introduction to GPL language, an explanatory page for digital
TTL chips and tutorials on how to solder and how to open your console
and PE-box.

These pages are meant to load fast over a phone line: no fancy graphics,
no pictures inserted in the text and (God preserves us) no frames! In
addition, the whole site is available for [download](download.htm#site)
as .zip files.

This site is currently hosted by the University of Sheffield, U.K.  It
used to be hosted by Stanford, but this changed when I moved to
Sheffield. So if you've bookmarked the Stanford site, please replace it
with 
height="112" />Now folks, I hate to brag,but I must admit that it feels
good to be recognized by one's peers. So let it be known that this site
was granted the Edgar Mauk Award in the catagorie \`TI Websites\`, at
the European TI-Treffen in Gent (Belgium), on October 7th 2000. Click
[here](http://www.ti-99.nl) for details.
height="117" />

In 2003, it was granted the Jim Peterson Award in the category
"Community Service".
data-align="left" alt="RTP award" />
And in 2005, it was selected for the Rob Tempelmans Plat award, a joint
initiative from the Chicago TI users group and the Dutch TI
Gebruikersgroep to honor the best internet sites for the TI-99/4A
community.

###

Below is the site map. Note that some pages may be listed under more
than one header.

##  The TI-99/4A console

[General architecture](architec.md)
[Pinout of ports and chips](pinouts.md)

**Processors**
[TMS9900](tms9900.md) CPU
[TMS9901](tms9901.md) Programmable system interface
[TIM9904](tim9904.md) Four-phase clock generator and driver
[TMS9918A](tms9918a.md)  (or  TMS9928A or TMS9929A) Videoprocessor
[TMS9919](tms9919.md) Sound generator
[TMS5220](speech.md) Speech synthesizer

**Memory**
[ROMs](roms.md)
[GROMs](groms.md)
[Scratch-pad RAM](padram.md)
[VDP RAM](vdpram.md)

**Miscellaneous**
[Keyboard](keyboard.md)
[CRU](cru.md) (communication register unit)**  **
[Interrupts](ints.md)
[Cassette tape interface](cassette.md)
[TTL chips](ttl.md) featured in the schematics on this site

###  Peripheral cards

[The Peripheral Expansion Box](pebox.md)
[The TI disk controller. Part I: Theory, FDC](disks.md) , [Part II:
Card, drives](disks2.md)
[The TI RS232C interface card](rs232c.md)
[The german 128K GramKarte](gramkart.md)
[The P-Gram card](pgram.md)
[The 32K memory expansion card](mem32k.md)
[The SupeRAMs memory card](superams.md)
[The p-code card](pcode.md) , [Part II: the p-system](psystem.md)
[The Horizon ramdisk](horizon.md)
[My IDE interface card](ide2.md), [PartII: Software](ideal.md) , ([Old
version](ide.md))
[The MBP clock + ADC card](mbp.md)
[The ForTI sound card
](forti.md)[My USB/SmartMedia card](usb.md) , [Part II:
Software](usb2.md) , [Part III: Build it](usbhowto.md) (see below for
related software).

###  Cartridges

[Editor-Assembler](ea_mod.md)
[TI-Writer](wr_mod.md)
[Mini memory](mm_mod.md)

###  Extra hardware

[TI joysticks](joystick.md)
[PC joysticks adapter](joystick.htm#Analog%20joysticks)
[WiPo mouse 99](wipmouse.md)
[Navarone's Widget](widget.md)
[GROM reader for the PC](gromread.md) (Courtesy M. Comstedt)

##  Upgrading your TI-99/4A

**Tutorials  **
[Do-it-yourself primer](howto.md) (Soldering, making boards, tools,
where to buy, etc).
[Open-console surgery guide
](surgery.md)[PE-box surgery guide
](pesurg.md)[Index of modifications

](howto.htm#Modifications)**Peripherals  **
[Adding a second RS232 card
](rs_card.htm#CRU%20modif)[Replacing card ROMs with
EEPROMs](eeproms.md)
[Multi-bases Widget](widget.htm#Modifying)
[Installing the speech synthesizer inside the
PE-box](speech.htm#in%20PEbox)

**Console modifications  **
[Powering the joystick port
](joystick.htm#modifying%20the%20port)[PC joysticks adapter
](joystick.htm#Analog%20joysticks)[Overclocking the
console](clockup.md)
[Supressing wait states](wait.md)
[Jeff Brown's interrupt mod](intmod.md)
[Replacing the console ROMs with EEPROMs](eeproms.htm#console)
[Disabling the console GROMs](nogroms.md)
[Console CRU interface](conscru.md)

##  Programming the TI-99/4A

**Languages  **
[Assembly language primer](assembly.md)
[GPL: the GROM language (I)](gpl.md) , [Part II](gpl2.md)
[The TI-Basic interpreter](basic.md)

**Tips & tricks  **
[Standard headers](headers.md): writing DSRs and Calls
[Calling GPL from assembly
](gplcall.md)[Using interrupts](ints.md)
[Floating point numbers calculations](reals.md)
[Using the internal timer](tms9901.htm#Timer%20mode) \*
[Sound generation](tms9919.htm#Programming) \*
[Speech synthesis](speech.htm#Operating) \*
[Direct access to the disk drive controller](disks.htm#FDCexamples) \*
[Scanning the keyboard](keyboard.htm#quick%20scan) \*
[Keyboard challenge (+ tutorial)
](tutor1.md)\* Part of another page

**Applications  **
[Blowfish encryption](blowfish.md)
[AVL trees](avltrees.md)
[Multitasking](tasks.md)
[Writing DSRs for the USB-SM card](usb_dsr.md) , [Part II: EEPROM
loader](usb_load.md) 

[Known TI-99/4A bugs](bugs.md)

##  Annotated pictures

[The Floppy disk controller](fdcard.md)
[The RS232c / PIO card](rs_card.md)
[The Horizon Ramdisk](hor_card.md)
[My IDE card](idecard.md)
[The Mini-memory cartridge](mm_board.md)
[The Speech synthesizer](synth.md)

##  Downloads

[Free software](download.htm#software)
[Disassembled listings](download.htm#disass)
[The whole site](download.md)
[How to download](download.htm#how%20to)

##  Feedback

[Comments, corrections](feedback.md)
[Information wanted](feedback.md)
[An interview with yours truly](inter.md)
[E-mail me](http://nouspikel.group.shef.ac.uk)

##  The TI-99/4A world

[Links](links.md)
[Bibliography](biblio.md)
[ti99-4a online user group
](http://groups.yahoo.com/group/ti99-4a/messages)[ti99-4a programming
group
](http://groups.yahoo.com/group/SWPB/messages)[comp.sys.ti
newsgroup](news://nntp/comp.sys.ti)

alt="The TI-99/4A Web Ring" />](http://99er.hispeed.com/tiring.html)
This [TI-99/4A Ring](http://99er.hispeed.com/tiring.html) site is owned
by [Thierry Nouspikel](http://www.nouspikel.com/home.md)
[Next
Page](http://www.webring.org/cgi-bin/webring?ring=ti994a&id=22&next) \|
[Skip It](http://www.webring.org/cgi-bin/webring?ring=ti994a&id=22&skip)
\| [Next
5](http://www.webring.org/cgi-bin/webring?ring=ti994a&id=22&next5) \|
[Previous](http://www.webring.org/cgi-bin/webring?ring=ti994a&id=22&prev)
\|
[Random](http://www.webring.org/cgi-bin/webring?ring=ti994a&id=22&random)
Want to join the ring? Get the
[information.](http://99er.hispeed.com/tiring.html)
