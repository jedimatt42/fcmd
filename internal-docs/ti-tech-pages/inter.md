# An interview with yours truly

The following is an e-mail interview I gave to Dan Eicher, who wanted to
publish it in the Chicago TImes Newsletter. I haven't seen the published
version, so I don't know whether Dan did any editorial changes (for
which he had my permission). What follows is the original answers I sent
him. I just added an few update, *in italics*, when things had changed
since January 2001.

And in case you are wondering what I look like, here is a [mug
shot](thn1.jpg).

Now, let's move to are Dan's questions:

[How and when did you get started with the TI?](#started)
[How did you make the jump from Basic to Assembler or Basic to GPL?
](#jump)[Have you ever worked with any other TIer's on a project?
](#collab)[How old were you when you wrote first assembly or GPL program
on the TI?
](#first%20program)[What made you want to write a GPL development
system, instead of say a Terminal Emulator or Pascal compiler?
](#why%20gpl)[What is your favorite application written in GPL?
](#fav%20gpl)[If you could go back in time, what change would have you
made to the TI OS or hardware?
](#OS%20changes)[Do you think, if TI would have created a GPL
development system - like the E/A package, we would have had more/better
software?
](#TI%20GPL%20pack)[What tools do you use to create your software?
](#tools)[What provided you with the spark to create your website?
](#website)[What are the major differences between your GPL development
system and the Weiand and Green Package?
](#diff%20GPL%20packs)[Why do you suppose to made such a poor loader and
how do your loaders improve upon it.
](#TI%20loader)[Do you know who designed the original GPL language?
](#who%20did%20GPL)[What do you think of the SNUG High Speed Gpl Card,
both in concept and implementation?
](#HSGPL)[If someone wanted to learn GPL, what route would you recommend
they take to get started?
](#learn%20GPL)[Is it my imagination, but does GPL based software do a
much better job at I/O handling then Assembly?
](#GPL%20IO)[Do you think GPL was the right choice for TI to write Disk
Manager II and / or Video Chess?
](#GPL%20cartridges)[What do you think about Monte Schmidts linker that
allows you to run GPL code from RAM instead of GRAM/GROM?
](#Monte%27s%20linker)[Have you ever considered using the load interupt
with your debug software?
](#LOAD%20ints)[Can you sum up how your debuggers work?
](#Debuggers)[What gave you the biggest feeling of accomplishment?
](#accomplishments)[What is your most memorable TI moment?
](#moments)[What was your favorite piece of TI hardware?
](#fav%20hardware)[What was your favorite piece of software that you
didn't write?
](#fav%20soft)[Who was/is your favorite TI personality?
](#fav%20person)[Were you active in any users group in Europe?
](#TI%20group)[Are there any software or hardware projects you are
currently working on?
](#projects)[A little about yourself! What are your hobbies? What is
your professional training?
](#yourself)[What is better/or worse about living in the states?](#USA)

#### D.E: How and when did you get started with the TI?

**Th.N:** It was around 1983, I was in med school doing a rotation in
the urological surgery department. The other rotating student was a
TI-99/4A fan and he told me so much about it that I decided to buy one.
He knew a place in his hometown where to buy a cheap console, so I
borrowed my sister's car, drove 40 miles and bought my first TI-99/4A.
When I brought it home, everyone was quite excited: it was the first
computer we ever had at home. My aunt donated a huge black and white TV
(the kind that still worked with tubes!) and I must admit it was quite
nice to work with such a big screen.

After a while, I got tired of the sluggish cassette tape interface and
decided to buy a PE-box. I purchased a second-hand, fully loaded box,
with memory expansion, disk drive and controller, RS232 card and p-code
card. I remember I paid around \$1000 for it, and at that time it was
considered a good deal!

A few years later, my console started acting up. The first time, I was
able to get it repaired through the store I purchased it from, but the
second time they considered it beyond rescue. So I placed a classified
ad in a local newspaper, but these guys misstyped my phone number in the
ad, and only a few people were smart enough to call the newspaper and
ask for the correct number. One of them was the kid I bought my new
console from (after surprising his mom topless in her backyard). Another
was Rudolf Scheffer then president of the CCTI, a swiss TI-99/4A
computer club. He offered me to join the club, which I did.

Unfortunately, it was not a very active club. Still, we had a
newsletter, a software library and sometimes good offers on hardware.
It's through the club that I bought the german 128K Gram-Karte.

When the CCTI drifted towards a PC and Amiga club, I found a new
TI-99/4A community: the newsgroup comp.sys.ti, which I'm still browsing
on a daily basis. I got my Horizon Ramdisk from somemone advertising it
on this newsgroup.

Finally, in December 1996 I moved to the USA and had to leave my TI
system behind (too heavy, and won't work on US power supply anyhow). I
got myself a PC and learned C++, but I was still missing my old
TI-99/4A. So after a year or two I bought a complete system for \$100.
They guy I got it from had been less carefull with it that I was with
mine, and probably let it rust in his garage for a while. As a result, I
often experience crashes and I still can't manage to get the disk drive
to read the last 10 tracks on any floppy! Of course, by that time my old
system was also beginning to rust in my dad's garage. So the next time I
went to Switzerland, I brought back my Ramdisk, Gramcard, speech
synthesizer and all I could pack into my suitcase. Then United lost the
suitcase!!! They finally found it, heavily damaged, but fortunately none
of the hardware had suffered.

####  D.E: How did you make the jump from Basic to Assembler or Basic to GPL?

**Th.N:** I always wanted to try my hand at assembly, but the guy I
bought my PE-box from wanted an extra \$400 for the cartridge and I
couldn't afford it. Fortunately, the p-code card came with an assembler,
so that's how I got started with this language.

Later on, once I had a Gram card, I cloned the Editor/Assembler module
in it (yeah I know it's illegal, but TI was done with the TI-99/4A
anyhow). I had no manual and had to figure out most of the tips and
tricks myself. This had some consequences on my programming style: for
instance I don't use R for registers. This is because you must specify
an option for the assembler to accept the R notation and I didn't know
about it!

I finally bought a book on assembly programming, but by that time I had
figured out most of what was in the book. A much more usefull reference
was instruction manual for MG's Explorer.

####  D.E: Have you ever worked with any other TIer's on a project?

**Th.N:** Not really. I have served as a beta tester, and others have
tested my softwares but I never embarked on any collaborative venture.
When I was a member of the CCTI, I tried to interest other members in
assembly language, my dream being to create a C compiler (I didn't know
of Clint Pulley's by then). I even wrote an article in our newsletter to
encourage people taking up assembly. I'm afraid I wasn't too
convincing...

I kind of regret it: working with guys like Micheal Becker would have
been a lot of fun, I assume. On the other hand, it's probably better for
the other guys that I didn't! You see, I tend to work on several
projects at a time, to leave one on the back burner when I get tired of
it, and to come back later... years later in some cases. Let's see, how
many projects fell from the back burner into the deep freezer by now? 1)
A music editor, 2) A graphic disk doctor, 3) A semi-graphic user
interface (featured in my GPL package, in the List Editor and in MISS),
4) An stud-poker playing program (with bluff capabilities, human
psychology appraisal and a self-improvement routine), 5) A multitasking
package, for both assembly and Extended Basic. And the list goes on.

*NB I actually completed and released the multitasking package. You can
find it on this site.*

In most cases once I have written the core routines, the tricky part of
the program, I get bored of doing the final polishing and move to
another, more interesting project. This bad habit would have driven any
coworker crazy.

####  D.E: How old were you when you wrote first assembly or GPL program on the TI?

**Th.N:** I'm not sure. Assembly must have been around 1985, so I was
24. GPL came later, since I had to write my own assembler for it.
Probably at age 26 or 27.

Which allows you to calculate that I am now 40, or will be on February
4. And you know what? I just love the idea. It looks like each decade in
my life was better that the previous one and I can't wait to see how
great the fourties will be!

####  D.E: You are probably now, best known for you terrific website. Prior to this,you were best know for your GPL development package. What made you want to write a GPL development system, instead of say a Terminal Emulator or Pascal compiler?

**Th.N:** When I bought my GRAM card, I was quite pissed off to discover
that it did not come with a GPL assembler. There were a few demo source
files, but the assembler costed extra money. I couldn't afford buying it
on top of what I spent for the card, so I decided to write my own
assembler. I deduced the syntax from the source files, from Heiner
Martin's book "TI-99/4A Intern" and by stepping through the GPL
interpreter with Millers Graphics' Explorer.

This first GPL assembler was fairly basic and clumsy, so a few years
later I set out to write a better one. Once I was almost done, someone
sent me the one by R.A. Green. This came as a shock for me! I must
confess that I would never have written mine if I had gotten my hands on
the RAG one first. As it was, I needed little work to finish mine, so I
decided to go ahead and release it.

The irony is, I never really used it much. The only two applications
that I ever wrote in GPL (that deserve to be called programs) are:

Padlock: a small utility that lets you protect a cartridge with a
password (not very secure though)

A GRAM-based disassembler. I'm using this one a lot: since it resides in
the GRAM card, it can disassemble the whole memory expansion, cartridges
and RAMBO banks, card DSRs, etc.

####  D.E: What is your favorite application written in GPL?

**Th.N:** That would probably be Extended Basic. But of course, that's
not pure GPL: there is quite a bit of assembly in this cartridge.

####  D.E: If you could go back in time, what change would have you made to the TI OS or hardware?

**Th.N:** Hardware: the data bus multiplexing from 16-bit to 2 times
8-bit. That's in my opinion the worst design mistake TI ever did with
this machine. I understand the historical context: the TI-99 was
designed around an 8-bit processor that was never produced, so TI
engineers took the 16-bit TMS9900 and tailored it down to fit their
design. They were smart enough to at least install the console ROMs as
16-bit memory, but that's about it. Why not go all the way and make the
TI-99/4A a real 16-bit computer? Beats me.

And while we are at it, why not make use of the 16 interrupts levels?
The TMS9900 has the required input pins, the TMS9901 the required output
pins. It was such a simple matter to connect them. And yet they decided
not to do it...

As for the OS, I think they did a pretty good job. What I would
definitely change is the way the interrupts are handled. There are 3
possible sources of interrupt in the system: the VDP, the peripheral
cards, and the internal countdown timer in the TMS9901. Even without any
hardware modification, it would be a simple matter to determine where
the interrupt came from and to react accordingly. Yet that's not what
the interrupt routine does. Instead, it relies on a flag set by the user
to decide either that all interrupts come from the timer, or that none
does! What's worse, the timer interrupt routine consistently overwrites
the return address, which means that you cannot return from such an
interrupt.

####  D.E: Do you think, if TI would have created a GPL development system - like the E/A package, we would have had more/better software?

**Th.N:** Not necessarily. The problem here is that GPL can only run in
special memory chips (GROMs) that were not readily available. Even if TI
had sold cheap GROMs by the bucket, together with the appropriate
programming device, it still would be inconvenient to develop a GPL
program. Can you imagine, burning a new chip each time you change a line
in your program?

What you need is GRAM, but there is no such thing. The so-called GRAM
cards contain regular RAM and a sophisticated circuit that mimics the
behaviour of a GROM, at least in part.

Now, if TI had released a GRAM card or an emulator like Monte Schmidts',
which lets you test your GPL program from standard memory, then maybe we
would have a lot of excellent GPL programs around. But I always
suspected that GROMs were part of a marketing plot by TI, to avoid
software piracy. So obviously, they were not going to promote GRAM
cards!

####  D.E: What tools do you use to create your software?

The good old TI assembler: it's still the best in my opinion.

I use the Funnelweb environment, which is much more user-friendly than
the Editor/Assembler cartridge. I just don't like their disk manager so
I use DISKU instead.

I sometimes use the RAG-linker, by R.A. Green.

Millers Graphics Explorer for debugging. Occasionally, my RIP 1.0.

And of course my own package when it comes to GPL :)

Now, there is a new exciting possibility around: Joakim Langlet has
written a TMS9900 assembler, linker and librarian that run on a PC
([www.seaview.se/index_eng.md)](http://www.seaview.se/index_eng.md).
You could write your assembly programs in a nice integrated development
environment, such as Microchip's MPLAB, assemble them at lightning speed
and transfer them to the TI-99/4A for test purposes via a connection
cable. I may well switch to this system in the future.

####  D.E: What provided you with the spark to create your website?

**Th.N:** A few years ago my youger sister and her husband offered me a
domain name (www.nouspikel.com) as a Christmas present. I put my picture
and my resume on it and then wondered, what else am I gonna put in
there? I started typing a few pages about things I found interesting in
the TI-99/4A: the interrupts, the CRU, etc. And then my anal
retentiveness kicked in, and I thought: "no way I can go public with
such a primitive site". So I went on typing in the info I had on the
various processors, and little by little I built a more comprehensive
site. I must admit that sometimes it's kind of boring to type in all
these technical details, but on the other hand it forces me to really
understand the manuals.

The site is still growing, mainly thanks to the help of readers that
send me the technical info I am still missing. It's currently hosted by
Stanford University and can be accessed there directly
([www.stanford.edu/~thierry1/ti99/titechpages.htm](http://www.stanford.edu/%7Ethierry1/ti99/titechpages.md))
or through my domain name
([www.nouspikel.com/ti99/titechpages.htm](http://www.nouspikel.com/ti99/titechpages.md)).
As the company that handles the domain name is based in the UK, the
first URL is probably faster for north-american users.

####  D.E: What are the major differences between your GPL development system and the Weiand and Green Package?

**Th.N:** All are based on the Weiand syntax and, as I said, the RAG
assembler is excellent.

My package offers a few extras, that you may or may not have a use for:

The assembler can produce relocatable code, to be loaded anywhere in
GRAM.

I included a lot of macros and pseudo-instructions to make the
programmer's life easier: for instance the FMT mode now supports text
mode.

The loader handles both GPL and assembly and links them together. This
makes easier to write hybrid cartridges and to include XML routines in
your program.

The latest version runs under Funnelweb and makes use of several nice
features of this OS, like loading scripts.

####  D.E: Why do you suppose TI made such a poor loader and how do your loaders improve upon it?

**Th.N:** My guess is, they wanted to keep it small so the user had as
much memory available for his program as possible. The price to pay was
to leave aside a number of nifty features provided by the assembler,
such a segmented programming.

This leads to another problem, the proliferation of loaders: one for
tagged-object files, one for EA5 format, one for tagged-object GPL, one
for GRAM-Kracker files, etc. The McGoverns did a nice job of integrating
about six assembly loaders in Funnelweb, but they did not address GPL.

My own attempts include two loaders:

A memory-image loaded, MILD, that can load EA5 files (including into
RAMBO banks or Gramcard banks), Gram-Kracker files, and even a splash
screen complete with music and moving sprites. MILD works from anywhere
in memory and will shifts itself aside if a file must be loaded in the
location where it currently resides. It can even overwrite itself in
case you really need all the memory available.

A (slightly pompously named) "Universal Loader" that loads tagged object
assembly and GPL files, library files, and supports loading scripts.

Yet I agree this is not perfect. I'd like to see support for dynamic
loading of program segments, in a "overlay" or "DLL" manner, i.e.
loading a chunk of code when your program needs it, wherever there is
room available.

*Note by D.E: FWIW - Jeff Brown was working on a loader he called the
chunk loader that would load large segments in any available memory.*

####  D.E: Do you know who designed the original GPL language?

**Th.N:** I haven't the faintest idea. I also wonder who came up with
the prefix "Graphic" since there is nothing so graphic in GPL, nor in
GROMs.

####  D.E: What do you think of the SNUG High Speed Gpl Card, both in concept and implementation?

**Th.N:** I heard a lot of good things about it, but I don't have one so
it's hard for me to make an opinion. Let's say that what I heard of it
makes my mouth water. If anybody has one for sale, by all means let me
know.

*NB I got one now, thanks to Richard Bell. It's a bit disapointing
hardware-wise because everything is handled by the FPGA chip: not much
for me to fool around with. But the card itself is great. Michael surely
diserves the many awards he got for this card.*

I got in touch with Michael Becker and asked him for schematics and/or
technical details, but he wasn't too keen about it. Too bad: the guy is
one of my personal heros. I mean, he and his buddies design a new card
every 2-3 years, can you imagine?

####  D.E: If someone wanted to learn GPL, what route would you recommend they take to get started?

Get a GRAM card, or possibly an emulator like Monte Schmidts (although I
never tried the latter).

Get a GPL assembler. Mine or R.A. Green's.

Read a description of the GPL language. There is one on my website,
others have been published.

Write your first GPL program. Keep it simple to start with...

If needed, you can use my Module Explorer program as a debugger. It may
also help you learning GPL, as it lets you see what happens when an
instruction is executed.

####  D.E: Is it my imagination, but does GPL based software do a much better job at I/O handling then Assembly?

**Th.N:** It's your imagination :) Actually, the GPL "I/O" instruction
is pretty clumsy. The only I/O that's easier in GPL is writing to the
screen. And then again, it only works in standard mode (32 columns,
etc), with all the VDP tables at fixed locations, etc.

####  D.E: Do you think GPL was the right choice for TI to write Disk Manager II and / or Video Chess?

**Th.N:** OK, there are good and bad reasons to use GPL. A really bad
reason is to turn a cartridge into a big dongle to prevent the user from
illegally copying a program. A typical example of this strategy is the
TI-Writer cartridge. All the cartridge does (apart from a tiny disk
catalog routine and storing foreing language characters and prompts) is
to launch the Editor or the Formater from disk. You don't need GPL for
that. As a matter of fact, I wrote a crack many years ago, in which a
tiny assembly program was doing the same job.

So are there good reasons to program in GPL? Since it's an interpreted
language, it is bound to be slower than assembly. On the other hand, it
is slightly more compact and easier to use (ok, the latter may not be a
good reason to choose a language).

But the main advantage of GPL is that it resides in a different memory.
As you know, the TI-99/4A address space is limited to 64K, of which
cartridges only get 8K (although some cartridges page the last 4K, but
let's not go there for now). On the other hand, each GROM base address
opens access to 64K of memory (40K if you don't want to overwrite the
console GROMs, 30K if you install 6K-GROMs every 8K, as TI used to do).
Now, the console ROM routines will scan sixteen GROM bases, which adds
up to 1 megabyte! And nothing prevents you from using more than 16
bases, provided that you don't put DSRs or CALLs in these. So, one good
reason to use GPL is to pack more software in a cartridge. Chess and
Extended Basic are good examples of this strategy. It's less clear for
Disk Manager: a priori, it could just as well be written in assembly.

####  D.E: What do you think about Monte Schmidts linker that allows you to run GPL code from RAM instead of GRAM/GROM?

**Th.N:** Sounds like a good idea. It lets you experiment with GPL
without having to buy/build a GRAM device. However, it's bound to have
limitations as far as the size of the program that you can write, since
both the program and the interpreter must fit in the memory expansion.

####  D.E: Have you ever considered using the load interupt with your debug software?

**Th.N:** Of course I did! That's the main idea behind my RIP debuggers:
trigger an interrupt that cannot be ignored, so the debugger gains
control no matter how badly locked up your program is.

####  D.E: Can you sum up how your debuggers work?

**Th.N:** RIP version 1 is a post-mortem debugger. You trigger it after
your program has crashed and it lets you inspect memory and registers to
determine what went wrong.

RIP version 2 has the same abilities, but in addition is also works like
a traditional debugger: it inserts breakpoints in your program and kicks
in when one of these breakpoints is reached. Since I wanted to use as
little memory as I could, it runs within the Horizon Ramdisk DSR space
and only needs about 30 bytes in the high memory expansion.
Unfortunately, when I moved to the USA and reinstalled it on my Ramdisk,
I discovered a nasty bug that makes user input quite awkward. I've just
been too busy (or too lazy) to fix it yet

RIP version 3 will be a remote debugger. You load a tiny routine
anywhere in memory, and connect your TI-99/4A to the debugging computer
(likely a PC) via a connection cable that could go either to the PIO
port, the cassette port, or the joystick port. Then you run your program
on the TI-99/4A and debug it from the PC. At least that's the idea, I
have barely started working on it.

Module Explorer is a lift of the GPL interpreter, modified so that you
can execute one instruction at a time, edit the memory, etc. Its
look-and-feel is purposedly reminiscent of Millers Graphics Explorer and
it's one of the softs I'm most proud of.

####  D.E: What gave you the biggest feeling of accomplishment?

**Th.N:  **That was probably the day I realized that my IDE card was
working. I always wanted to create a hard-disk interface for the
TI-99/4A, but did not really believe I could succeed. Of course, the IDE
standard made things quite easy, by having the controller inside the
drive instead of on the interface board.

####  D.E: What is your most memorable TI moment?

**Th.N:** Each time I get my hands on a new piece of hardware, or
discover a really nice software, is a memorable moment.

####  D.E: What was your favorite piece of TI hardware?

**Th.N:** The Horizon Ramdisk. It's an extremely valuable piece of
hardware that saved me a lot of time. Its design is very elegant in its
simplicity. And ROS 8.14, the operating system, is just wonderfully
written.

#### D.E: What was your favorite piece of software that you didn't write?

**Th.N:  **Millers Graphics Explorer, by D.C. Warren. It's the king of
all debuggers. Actually it's more than a debugger, it's an emulator: it
emulates the TMS9900 microprocessor on the TI-99/4A itself. This
brilliant design lets you step through code in ROM (where a regular
debugger cannot place breakpoints), set breakpoints on VDP or GROM
memory addresses, etc. The manual is beautifully written and contains a
lot of extremely usefull information, a tutorial, etc. Last but not
least, it comes with an interesting protection scheme, that's quite
educational to crack. Yes, I hacked it! Not for the money since I
legally bought it, but just for the learning experience. And I'm glad I
did it: after all these years, the floppy is so worn out that Explorer
won't load of it. But the hacked version sits in my Ramdisk and loads in
less than 2 seconds!

Some of my other favorite programs include:

DISKU, by John Birdwell. One of the best disk managers I'm aware of.

The RAG linker (and basically anything written by R.A. Green).

Advanced Diagnostics. A nice disk fixer by Millers Graphics.

Funnelweb by Tony and Will McGovern. Originally a word processor, it's
the closest thing from an operating system that you can get without
rewriting the console GROMs.

As you see, there are no games on my list as I'm not very found of
computer games. I played a few with my 6 years old nephew, and we both
liked Parsec, Slymoids and some of the Atari games, but my heart goes to
utilities, not games.

####  D.E: Who was/is your favorite TI personality?

**Th.N:  **Ok, so you want my list of personnal heros? There are many.
For software matters, any of the guys who wrote the above programs: D.C.
Warren (Explorer), R.A. Green (linker, assembler, GPL assembler, GUMS,
etc), Tony and Will McGovern (Funnelweb), Barry Boone (SoundFX, Archiver
III), John Birdwell (DISKU), Clint Pulley & Winfried Winkler (small C),
Joakim Langlet (PC-based assembler and linker), etc.

For hardware matters, probably Michael Becker. He's just so productive.
And the Horizon Ramdisk designer: Ron Gries.

For TI community matters, Rich Polivka, because of his great website.

#### D.E: Were you active in any users group in Europe?

**Th.N:** Yeah, I was an early member of the Swiss CCTI (computer club
TI-99/4A). Unfortunately, after a few years it became a PC/TI club, then
an PC/Atari/TI club. I still remember the general assembly when we voted
to create the PC section: I had the distinct feeling that the TI-99/4A
section was doomed, but there was no point going against the flow. And
of course, that's exactly what happened: as far as I know the club still
exists, but is now headed by Eric Scheffer (the president's son) and is
almost exclusively geared towards PC. Not that there is anything wrong
with that, of course.

####  D.E: Are there any software or hardware projects you are currently working on?

**Th.N:** Hardware: as already mentionned, I have designed an IDE
interface card. It can handle two 128-gigabyte drives, and has a 512K
SRAM chip on board to store the DSRs and buffer the opened files (the
rest of the memory is available to the user via the RAMBO protocol). It
also carries a real-time clock, so as to time-stamp the files. The clock
chip I selected has 4K of battery-backed RAM, which I'm using to store a
boostrap program that loads the DSRs into the SRAM upon power up.

My next project will probably be a memory expansion board. But first I
have to complete yet another ongoing project, not related to the
TI-99/4A: a sensory chessboard, controlled by a PIC microprocessor, with
LCD screen, voice chip, clock, flash-memory, etc.

Software: I'm busy writing the DSRs for the IDE card, but once this is
done I really want to release this multitasking package of mines.
Possibly also the semi-graphic interface, if I can muster the energy.
Then there will be the remote debugger. Then I'd like to make another
attempt at a real good loader, that would handle segment programming,
memory expansion boards, GPL, etc.

*NB. Both the IDE software and the multitasking package are now ready
and available from this site.*

####  D.E: A little about yourself! What are your hobbies? What is your professional training? What is better/or worse about living in the states?

**Th.N:** I'm a swiss citizen. This means I was born in Switzerland, not
Sweden, OK? And no, it's not in southern Africa: that would be Swaziland
(sorry, couldn't resist).

I was trained as a MD, but never actually practiced medicine, since I
found out in med school that I loved research much better than clinics.
So I got myself a Ph.D. in biology and I'm currently working on DNA
repair at Stanford University. And I can tell you guys, research is the
most fascinating game there is. I'd do if for free, if I had to (but
don't tell my boss).

My hobbies include, as you might have guessed, computer programming
(TI-99/4A and PC. I hate these unreliable MacIntosh) and digital
electronics.

When I moved to the States, I took up martial arts: first Aikido and
japanese swordsmanship, more recently Kenpo. I try to train 5 or 6 days
a week, which tends to keep me away from my TI-99/4A, unfortunately.

I'm very found of classical music, as a listener not a performer. I
tried for years to learn how to play the piano, but was really miserable
at it. Maybe one day I'll try the guitar... In the meantime I'm learning
japanese, just for the fun of it.

*NB I did try the guitar... with not much success but I'm still working
on it.*

####  D.E: What is better/or worse about living in the states?

**Th.N:** What do I like about living in Silicon Valley?

There are plenty of electronic stores around, where you can find any
component you can dream of.

California weather beats Switzerland any day.

American politics is much more funny that swiss politics.

What I don't like:

My girlfriend is 6,000 miles away. But we're working on that. *NB We
did: she is now my wife and we are living together, in complete bliss.*

You can't go anywhere without a car. And it always looks like everyone
else wants to go there too.

You can't find decent yogurt in any store. All they have is made of
low-fat milk (or worse, fat-free milk) and is tainted with starch,
gelatin, pectin, guar gum, you name it. But at least this is now fixed:
my girl offered me a yogurt maker for Christmas.

[Back to the TI-99/4A Tech Pages](titechpages.md)
