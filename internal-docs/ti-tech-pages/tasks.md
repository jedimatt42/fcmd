# Multitasking the TI-99/4A

This page introduces the concept of multitasking and describes a package
that I put together to let you write multitasked programs in the
following languages:

- TI-Basic (with the Editor/Assembler or Mini-memory cartridge plugged
  in)
- Extended Basic
- Assembly language
- GPL

The package can be downloaded as a zip file: [tasks.zip](tasks.zip). It
should expand into three "disks" files on your PC: one contains the
Basic & Extended Basic sub-package, the second contains the Assembly and
the third the GPL sub-package. (Due to lack of space one of the assembly
files, `TASKS/EQU`, was placed on the GPL disk). See my
[download](download.md) page for instructions on how to transfer them
to your TI-99/4A.

In addition, the zip file contains a Winword file called
"multitasking.doc" which is the reference manual for this package. The
rest of this page is basically an "HTML-ized" version of said manual.

**An introduction to multitaking  **
[Contents of the package
](#Package)[General principles
](#principles)[Cooperative vs preemptive
](#coop%20vs%20preemptive)[Critical sections
](#critical)[Task priority
](#priority)[Tasks synchronization
](#Synchro)[Instances
](#Instances)[Using the schedulers](#Using%20schedulers)

**Multitasking Basic and Extended Basic  **
[Introduction
](#Basic)[Tasks synchronization
](#sync)[Critical sections
](#critic%20&%20endcri)[I](#Basic)[nstances and local
variables](#instances&local)

**Multitasking Assembly language  **
[Introduction
](#Assembly)[Setting priorities
](#set%20priority)[Task synchronization
](#task%20sync)[Critical sections
](#critical%20&%20coop)[Switch-time routine
](#switch-time)[Instances & local memory
](#instances&local%20mem)[TCB: Task control blocks](#TCBs)

**Multitasking GPL  **
[Introduction
](#GPL)[Context saving
](#context%20saving)[XML routines](#XMLs)

##  Introduction to multitasking

Multitasking is the art of executing simultaneously several programs, or
executing several lines of execution within the same program, which is
called multithreading. True multitasking is only possible on computers
that have more than one processor. On most home computers there is only
one processor, so only one task can run at a time. In such a case, the
best substitute for true multitasking is time-sharing, i.e. each task
will run for a fraction of a second, then the computer switches to the
next task. When switching occurs 50 times per second or so, the human
user is under the illusion that tasks run simultaneously (except that of
course, each task runs more slowly than if it were the only task on
board).

### Contents of this package

This multitasking package consists in four schedulers, i.e. programs
that distibute execution time among several tasks. One scheduler is to
be used with Basic with the Editor/Assembler or the Mini-memory
cartridge plugged in, the second with Extended Basic, the third with
assembly programs and the last one with GPL. All can perform either
preemptive (recommended) or cooperative multitasking. They come with a
set of demo programs that are written as a tutorial, and that you should
examine in sequence.

To be honest, there are only two different programs: the Basic and
Extended Basic schedulers are one and the same. They just have different
file formats and minute differences. Similarly, the GPL scheduler is
only a thin wrapper around the Assembly one.

Also in the package is the present file, a reference manual that
describes each scheduler in all excrutiating details. It may therefore
look a bit intimidating. Thus, I would advise you to follow this
strategy:

1.  Finish reading this intoduction, upto the point marked "enjoy it".
2.  Pick either the Basic or the Extended Basic scheduler and run trough
    the demo programs. They are arranged in sequence as a tutorial, so
    you should take them in order. List each program and study it till
    you understand how it works.
3.  Read the Extended Basic section of this manual. Go back to the
    tutorial if needed, and see if now you understand this or that point
    more clearly.
4.  Experiment with writing your own multitasked Basic programs. Keep
    them simple to start with...
5.  Do the same for assembly language. The assembly scheduler is more
    complex, but gives you a finer control than the Extended Basic one.
    You may have to move back and forth between the tutorial and the
    manual (i.e. to multitask yourself!). Make sure you do the exercises
    before you move to the next demo (don't cheat: if you can't come up
    with a solution, go through the manual before you look up the answer
    at the bottom of the demo file).
6.  Once you mastered assembly multitasking, read the GPL section of
    this manual. It will appear very easy for you, since all it does is
    basically to call the scheduler's assembly routines in a
    GPL-compatible way. There is only one demo file for GPL that shows
    how to load the scheduler from GPL and how to call the various
    assembly routines. A good exercise would then be to translate the
    assembly demo files into GPL...

###  General principles

Multitasking is as easy as 1-2-3!

1.  Initialise multitasking: load the scheduler, call the INITSK
    routine.
2.  Create tasks: use the FORK routine.
3.  End tasks: use the KILL or DIE routines.

Some more sophistication is provided for your convenience:

1.  Tasks can be assigned a priority that affects the way they are
    scheduled.
2.  Semaphores can be used to synchronize tasks with respect to each
    other. Use the GRAB, DROP, WAIT, RAISE, and BARIER routines.
3.  Tasks can save the local context (VDP address, GROM address, local
    memory, etc) when switched off, and retrieve it when switched on.
4.  The same routine(s) can be run within several tasks, without each
    instance interfering with the others.

###  Cooperative versus preemptive

There are two ways to implement time-sharing: cooperative multitasking
or preemptive multitasking. With cooperative multitasking, each task
decides when it's time to switch, and does so by returning control to a
scheduler, which sets up the next task. Typically, this is done by
calling the YIELD routine.

With preemptive multitasking, the scheduler snatches control away from
the current task and forces a task switch whenever it wants to. The
advantage is that a sloppy task, that never returns control to the
scheduler, will not stall the whole system, as it would with cooperative
multitasking. Typically, preemptive multitasking is achieved with
interrupts. On the TI-99/4A, the VDP interrupt is very convenient for
that purpose.

The TI-99/4A has two tasks running in parallel (yeah, it already has
multitasking!). One task is the main line of execution: title screen
==\> main menu ==\> whatever cartridge you select ==\> whatever program
you run. The other task is called 60 times per second by the VDP
interrupt service routine: it can move sprites in auto-motion, play a
sound list, test the \ key, and time-out the screen.

So is this cooperative or preemptive multitasking? Well, it depends
whether interrupts are enabled or not. In assembly language they are
generally disabled (TI recommends that you do so). If you want to use
the second task, you must briefly enable interrupts with a LIMI 2, LIMI
0 loop. Under these conditions, assembly language is cooperative. By
contrast GPL features preemptive multitasking, as the GPL interpreter
briefly enables interrupts in between each instruction. And so does any
language that is interpreted by a program written in GPL (such as Basic
and Extended Basic): interrupts can occur anywhere whithin a statement.

###  Critical sections

In a preemptive multitasking situation, there are cases when you
absolutely do not want a task to be interrupted. For instance, if you
are performing time-critical operations, such as writing to a disk. A
good scheduler lets you define "critical sections" within your program
and will never interrupt a task that has entered such a section.

In essence, a critical section is an island of cooperative multitasking
in a sea of preemptive multitasking.

###  Task priority

By assigning a priority to a task, you affect the way it will be
selected by the scheduler. When it is time to switch tasks, the
scheduler will preferentially select a task with a high priority to run
next.

There are two basic strategies to implement priority:

The task with the highest priority is always selected. No task will run
as long as another task with a higher priority is able to run (i.e. is
not waiting for a semaphore).

The probability that a task is selected depends on its priority. The
higher the priority, the more often it runs, but tasks with lower
priority also have their chances.

The Basic/Extended Basic scheduler is based on the second strategy with
255 levels of priority: the higher the level, the more likely a task is
to be selected for running. The Assembly/GPL scheduler implements both
strategies. There are four priority classes, and within each level 64
different priorities. The assembly scheduler only runs the tasks in the
currently highest class. It dispenses execution time among them
according to their priority withing the class. In other words, classes
implement the first stategy and within-class priority implements the
second.

###  Task synchronization

Not all task are completely independent from each other. In fact, it's
quite common that you will want a task to wait for another. For
instance, a printing task should wait until data is available to print.
Conversely, the data-preparing task should not feed them to the printing
task faster than they can be printed.

Semaphores are variables that can be used by a task to send a message to
another. What's so special about them? Nothing: they are numeric
variables like any other. The only trick is that you access them via
special routines that are immune to task switches. This is required so a
task switch does not change the value of a semaphore between the moment
you check it and the moment you modify it.

####  Mutex semaphores

The simplest kind of semaphore is the "mutex" or mutually exclusive
semaphore. You can think of it as a flag that can only be grabbed by one
person at a time. Once a task has grabbed the mutex, any other task that
wants to grab the same flag will have to wait until the first task has
dropped it. The GRAB subroutine is serving this purpose: a task that
calls it will be suspended (and control transfered to another task) if
the semaphore it wants is not available. Once it finally is, the task
will resume and will be the unique owner of this mutex semaphore. Once
the task is done, it should release the mutex with the DROP subroutine.

There are other subroutines that let you affect semaphores: WAIT waits
for a semaphore to be free, but does not grab it when it is. RAISE is
used to mark a semaphore as busy (i.e. to raise the flag), regardless of
whether it was free or not. With RAISE, the task will never be
suspended, even if the semaphore was already grabbed.

####  Numeric semaphores

Not all semaphores are simple mutex flags. It is possible to have
numeric semaphores, i.e. semaphores that can be grabbed by an arbitrary
number of tasks. Once this number is reached, any additional task that
tries to GRAB the same semaphore will be suspended. Its execution will
resume once one of the current tasks DROPs the semaphore. As you can
see, a mutex is just a special case of semaphore for which the maximum
count is one.

An example of numeric semaphore would be a sound playing routine. Since
the TI-99/4A has three sound generators, there could be upto three tasks
that each play a sound. If a fourth task wants to access the sound
generator, it will have to wait until one of the other three tasks is
done. Several of the demo files that come with the assembly scheduler
are based on this example.

####  Barriers

A barrier is kind of the opposite of a counting semaphore: it is closed
until a predefined number of tasks have reached it (i.e. called the
BARIER subroutine), then it opens and releases all the waiting tasks.
This may be usefull for instance when exiting your program: you want to
make sure that every task has completed its job. Or think of a game in
which the player must complete several distinct missions before he can
proceed to the next level.

###  Instances

There are cases when several tasks execute the same portion of your
program. Think of a Pacman game for instance: there are three "bad guys"
who each follow the same basic strategy and try to catch the human
player. They only differ by their colors and current screen location. It
seems silly to write three identical routines to handle them. A better
solution is to write only one routine, and run it in three different
tasks. It is said that you now have three "instances" of this routine.

The main problem with "instanciation" is that, since tasks share the
same code, they will affect the same variables (e.g. the screen
position, in the Pacman example). That's clearly not acceptable and
there should be a way to let each task have its own copy of the
variables.

Come to think of it, Basic and Extended Basic both use a value stack in
VDP memory, on which they save the return point of a `GOSUB `subprogram,
the parameters of a `FOR-NEXT` loop, etc. Clearly, this is something
that should be private to each task. And in GPL, you may want each task
to have its own copy of the current GROM address, of the subroutine
stack, etc. Globally, this is known as "context switching".

The schedulers in this package handle context switching automatically.
In addition, they provide support for you to define extra variables that
you want to be local to a task. This is done with the LOCAL subroutine
in Basic, and by modifying the task control block in assembly.

###  Using the schedulers

As mentionned above, this package contains several schedulers. Although
the mechanisms are the same in (Extended) Basic, in GPL, and in
Assembly, the syntax is of course different. So from now on, each
scheduler will be discussed separately.

At this point, I would advise you to go ahead and try multitasking in
(Extended) Basic. Just go through the demo files in order, their
contents are listed hereafter. The demos can be run either in TI-Basic
with the Editor/Assembler or Mini-memory cartridge plugged in, or in
Extended Basic. Just make sure that you load the proper scheduler:
TASKXB/O for Extended Basic and TASKBA/O for TI-Basic (together with
MMR/O if you're using the Editor/Assembler cartridge). You should find
the corresponding `CALL LOAD` around line 150 in each demo file. Be
aware that Extended Basic is awfully slow at loading assembly...

Enjoy it!

------------------------------------------------------------------------

Multitasking TI-Basic and Extended Basic

The following files are provided with the package:

    Filename        Type    Contents
    TAKSBA/O        DF80    TI-Basic scheduler
    MMR/O           DF80    Extra routines for Editor/Assembler
    TASKXB/O        DF80    Extended Basic scheduler
    TASKDEMO1       Prog    Tutorial: Cooperative multitasking
    TASKDEMO2       Prog    Tutorial: Preemptive multitasking
    TASKDEMO3       Prog    Tutorial: Critical sections
    TASKDEMO4       Prog    Tutorial: Priorities
    TASKDEMO5       Prog    Tutorial: Mutex semaphores
    TASKDEMO6       Prog    Tutorial: Numeric semaphores
    TASKDEMO7       Prog    Tutorial: Barriers
    TASKDEMO8       Prog    Tutorial: Local variables (numeric)
    TASKDEMO9       Prog    Tutorial: Local variables (strings)
    TASKDEMO10      Prog    Tutorial: Local value stack

The scheduler is the same for both languages, however it comes in two
different files. TASKXB/O is for use with Extended Basic, TASKBA/O with
TI-Basic and either the Editor/Assembler or the Mini-memory cartridge.
This is because Basic and Extended Basic have different error numbers so
I had to adapt the error codes in each version of the scheduler. Also,
TAKSBA/O comes in a compressed format, which is smaller and faster to
load but that Extended Basic cannot handle.

The problem with Editor/Assembler is that it does not provide any
routine to read or modify Basic variables, so I had to add these in the
MMR/O file (I lifted them directly from the Mini-Memory ROMs, hence the
filename). If you are using the Mini-memory, comment out the
`CALL LOAD("DSK1.MMR/O")` lines with a REM in the demo files.

If you are using Extended Basic, delete the line loading MMR/O and
replace TASKBA/O with TASKXB/O in the next line.

###  Loading the scheduler

    100 CALL INIT
    110 CALL LOAD("DSK1.MMR/O") with Ed/Assm only
    120 CALL LOAD("DSK1.TASKBA/O")     or      CALL LOAD("DSK1.TASKXB/O")

Either scheduler defines the following routines:

    INITSK  Initialize multitasking
    FORK    Create a new task
    KILL    Kill a task
    DIE     Kill the current task
    GRAB    Grab a semaphore
    DROP    Drop a semaphore
    BARIER  Wait at a barrier
    RAISE   Raise a semaphore
    WAIT    Wait for a semaphore
    SETSEM  Set the value of a semaphore
    CRITIC  Enter critical section
    ENDCRI  Leave critical section
    YIELD   Yields to another task
    LOCAL   Reserve local variables
    ENDTSK  Terminate multitasking operations
    CFGTSK  Modifiy default configuration

###   Initializing multitasking

    120 REM There must be a comment line here
    130 CALL LINK("INITSK")

Calling INITSK initializes the scheduler for multitasking operations.
This must happen before any call to another routine in the scheduler
(except for CFGTSK, if present).

The comment line is necessary, because that's where the scheduler jumps
to switch tasks. INITSK will thus be called in between each task, but
only the first time does it really initialize the system. The rest of
the time, it's just a signal for the task switch to occur.

####  How it works

The first time INITSK is called, the scheduler installs an interrupt
hook. Within the (Extended) Basic interpreter, interrupts are allowed
briefly before each keyword is interpreted. If this keyword is
interpreted by an assembly routine in ROM, this is the only chance for
the scheduler to gain control on this line. Exemples of such keywords
include: DATA, DEF, DIM, ELSE, END, IF, GO, GOSUB, GOTO, NEXT, REM,
RETURN, ON, OPTION, STOP,and SUB.

Most other keywords and CALLed subprograms are interpreted by GPL
routines in GROMs. This is of course much slower because GPL itself is
interpreted by assembly routines. However, interrupts are briefly
enabled before almost each GPL instructions, which means that
GPL-interpreted keywords offer many opportunities for the scheduler to
take control anytime whithin the instruction. Of course, switching tasks
before an instruction line is completed is a big no-no, so the scheduler
does it in a more subttle way.

When the scheduler gains control through an interrupt, it decides
whether it's time to switch tasks. Since GPL-interpreted keywords may be
quite slow, the scheduler will not switch tasks before at least two
Basic lines have been executed. Otherwise, the system could spend more
time switching tasks than actually running them! On the other hand,
assembly-interpreted keywords are very fast so it is quite possible that
many will be executed before an interrupt occurs.

Once it decides to switch tasks, the scheduler saves the current line
number and replaces it with the number of the REM line that's before the
call to INITSK. When (Extended) Basic completes the current line and
moves to the next, it actually lands on the line with the
`CALL INIT("INITSK")`. Entering INITSK tells the scheduler that tasks
can now be switched safely, i.e. that we are not in the middle of an
instruction.

However, there are instructions that do not move to the next line (e.g.
`IF`, `GOTO, GOSUB, NEXT`, etc.) so obviously INITSK will not be called
after such an instruction. (Which is good since execution may not
necessarily continue with the next line, when the task returns to such
an instruction). The scheduler thus keeps changing the line number so as
to point at the REM line, until the program finally jumps to it. This
complicated mechanism is necessary to ensure that the current line will
always terminate before switching, and that the next line will be the
one to execute next.

Take-home messages:

1.  Switching always occurs in between two lines. So compound statements
    in Extended Basic (e.g. `I=I+1 :: PRINT I :: PRINT "Test"`) will
    never be interrupted.
2.  The minimal "time slice" for a task is two lines (although you can
    change that with CFGTSK).
3.  Task switching will never occur just after a line that contains a
    `GOTO`, a `NEXT`, a `GOSUB`, or any instruction that causes a jump.

Note: If you TRACE the program, you may notice that some lines are
listed twice (just before and just after a task switch), although they
are actually executed only once. This is because the tracing routine
outputs the line number before the Basic interpreter figures where the
line is in memory, from its line number. Since the tracing routine is
written in GPL, it is possible that the line number will be changed
after it was placed on screen, but before the line was located and
executed. This line will only be executed when the task resumes. In
other words, TRACE is outputing the wrong line number, but don't worry:
everything happens as it should.

###  Terminating multitasking

    500 CALL LINK("ENDTSK")

Because the scheduler hooks the interrupt routine, we must unhook it
when we are done. Note that this should be done automatically when the
program terminates, but it's good practice to call ENDTSK anyhow. If you
\ the program, call ENDTSK, then INITSK to restart it.

###  Changing the default configuration

    100 CALL LINK("CFGTSK",TYPE,BUFFER,TASKS,SLICE)

This optional command lets you modify the default configuration of the
scheduler. If present, it must be placed at the top of your program,
just after the CALL LOAD that loads the scheduler file(s). All
parameters are optional, but you cannot leave gaps (e.g. define BUFFER
without defining TYPE).

**TYPE** is the type of multitasking desired: 0 means preemptive, a
non-zero value means cooperative. The default is preemptive.

**BUFFER** is the number of bytes to reserve for the local buffer. This
buffer is used to save the Basic value stack and local variables for
each task. The default size is 1152 bytes. Passing 0 leaves the current
value unchanged.

**TASKS** is the maximum number of tasks that can be created (including
the main program). The default is 16. Zero means no changes.

**SLICE** is the minimum number of lines that should be executed within
a time slice, i.e. before a task switch occurs. The default is two,
which I find the most convenient. Passing 0 means no changes.

CFGTSK may cause the following errors:

- If any of these value not in the range -32768 to 65535, you will get a
  "Bad argument" error.
- You may get a "number too big", if BUFFER or TASK is obviously crazy.
  However, the scheduler may miss some subttle points (such as extending
  the buffer beyond the limit of the low memory expansion).

Cautionary notes:

SLICE represents an offset from the number of the line in which the
tasks is entered, **not** the number of lines actually executed. Which
means that, if you have a 3-line FOR-NEXT loop, and you set SLICE as 4,
this loop will never be interrupted! Conversely, if the first line
executed is a GOTO that lands 50 lines down your program, chances are
that a switch will occur, even if SLICE was set at a high value. This is
because the scheduler has no way to know how many lines where executed
in between two interrupts. It only knows how far down from the original
line the program went.

Be aware that changing BUFFER and/or TASKS, changes the amount of memory
that must remain available just after the scheduler program. The file
TASKBA/O (or TASKXB/O) reserves the default amount, but if you change it
you risk overwriting any assembly file that you may load subsequently.
So either load the scheduler last, or load a dummy file containing only
a BSS statement with the additional size needed, or use `CALL LOAD` to
modify the loader's pointers. Hmmm, I guess the easiest is to load the
scheduler last...

###  Creating a new task

    150 CALL LINK("FORK",LINE,PRIOR,NESTING,ID)

This command is used to create a new task. The last 3 parameters are
optional.

**LINE** is the line number where the new task is to start. The parent
task will continue normally with the next line.You could also use a
constant instead of a variable: `CALL LINK("FORK",200)` spawns a child
task at line 200. Just be carefull when you `RESEQUENCE `your program:
the target line may have changed number...

**PRIOR** is an optional parameter, that specifies the task priority.
This must be a number between 1 and 255. If it is 0, or if it is omited,
the priority will be the same as that of the parent task. Here also, a
constant could be used. A task priority affects its chances of being
selected during a task switch: tasks with higher priority values run
more often. The priority of the main program is arbitrarily set as 16.

**NESTING** is an optional parameter, constant or variable, in the range
0 to 255. It indicates the maximum number of entries to be saved from
the value stack when the task is switched off (and restored when it is
switched on). If this parameter is not specified the default value will
be that of the main program: 9 entries. Stack entries are used by the
`GOSUB` (1 entry) and `FOR-NEXT` statements (3 entries in Basic, 4 in
Extended Basic). The number to reserve depends on how many levels down
such statements can be nested in a task. For instance, a `FOR-NEXT` loop
inside a subprogram called from a `GOSUB `located inside another
`FOR-NEXT` requires 3+1+3 = 7 stack entries in Basic and 4+1+4 = 9 in
Extended Basic. Each entry occupies 8 bytes in the scheduler's buffer.

**ID** is yet another optional parameter. Contrarily to the previous
ones, it must be a numeric variable, not a constant. This is because it
is a pure "output" parameter: FORK will return the ID number of the
newly created task in this variable. This number can be used by the
parent task to kill the child task, for instance.

FORK may cause the following errors:

- "Memory full" if there is not enough room in the scheduler memory to
  create the new task (e.g. if you are reserving a lot of nesting
  space), or if you create too many tasks.
- "Bad line number" if the line number specifed does not exist.
- "Number too big" if PRIOR or NESTING is greater than 255.

###  Terminating a task

There are two ways to terminate tasks: calling KILL or calling DIE.

#### Kill

    250 CALL LINK("KILL",ID)

**ID** is the ID number of the task to be terminated (obtained when
calling FORK). Zero stands for "current task" and is therefore
equivalent to calling DIE.

####  Die

    250 CALL LINK("DIE")

Kills the current task.

KILL and DIE may cause the following errors:

- "Number too big" if the ID entered does not correspond to a valid task
  (KILL only).
- "Can't continue" if you killed the only task that could run.

###  Synchronizing tasks

There are cases when you don't want two tasks to run at the same time:
when they both need to access the same resource, for instance. You can
cause two (or more) task to become mutually exclusive by using "mutex"
semaphores.

The scheduler contains 100 semaphores, which are internal variables used
to communicate between tasks. To achieve mutual exclusion, have each
task GRAB the same semaphore before it attempts to use the critical
resource (e.g. a printer). If the semaphore is free, the task will grab
it and continue normally. If the semaphore was already grabbed by
another task, the current task will be suspended. It will only resume
once the semaphore is freed by calling DROP, generally from the task
that had successfully grabbed it.

It is also possible to wait for a semaphore without grabbing it (WAIT),
to "grab" it without checking it first (RAISE) or to define its value
(SETSEM).

Actually, a mutex is just a special case of semaphore, that can only
take two values: raised (1) or dropped (0). But in fact semaphores can
have more than two possible states: legal values range from -128 to
+127. If the value of a semaphore is positive the task will hang on GRAB
or WAIT, if the value is zero or less the task will go through.

####  Grab

    800 CALL LINK("GRAB",SEM)

**SEM** can be either a constant or a variable. It is the number of the
semaphore to grab. The call will not return until the semaphore is free.
When it does, the semaphore will be marked as used by the current task
(i.e. its value is incremented by one).

GRAB can cause a "Number too big" error if the semaphore number is not
between 1 and 100.

Beware of deadlocks: when a task grabs semaphores \#1 then \#2, and
another task grabs semaphores \#2 then \#1, there is a risk that each
task ends up waiting for the semaphore grabbed by the other. Since none
of the two tasks ever runs, it is likely that neither semaphore will
ever become free! Also make sure a tasks never grabs the same semaphore
twice in a row, as it would end up waiting for itself!

A simple rule to avoid deadlocks is to grab semaphores in increasing
order, i.e make sure a task never grabs a semaphore with a number
smaller than (or equal to) the highest it has already grabed.

####  Wait

    800 CALL LINK("WAIT",SEM)

**SEM** is the semaphore number, just as with GRAB.

WAIT is identical to GRAB except the it does not raise the semaphore
once it returns. In other words, it makes sure the semaphore is free,
but it does not prevent other tasks from grabbing it later on.

This is a bit dangerous to do, since nothing guarantees than a task
switch will not occur just after the call to WAIT, transfering control
to another task that may grab the semaphore. When returning to the
current task, the semaphore will be in use, even though we went through
WAIT! In fact, it is to avoid precisely this kind of situation that
semaphores were invented...

####  Drop

    900 CALL LINK("DROP",SEM)

DROP releases a semaphore (i.e. decrements its value by one), whether or
not it was previously grabed by the current task. It is very important
that a task always releases any semaphore it may have grabbed before it
terminates. Other tasks may be waiting for one of these semaphores and
if you don't relinquish them the program may get stalled and abort with
a "Can't do that" error.

**SEM** is the semaphore number. If this number is negated, the
semaphore will only be decremented if its current value is positive (you
can't drop something that's already down, can you?).

####  Raise

    900 CALL LINK("RAISE",SEM)

RAISE increments the semaphore value by one. This will hang any task
that tries to GRAB it or to WAIT for it. Contrarily to GRAB, RAISE will
never suspend the current task if the semaphore value was already
positive.

**SEM** is the semaphore number. If this number is negative, the
semaphore value will not be increased if it is already positive (i.e.
you cannot "super-raise" such a semaphore).

####  Barrier

    900 CALL LINK("BARIER",SEM,VAL)

Barriers use semaphores in a way that allows tasks to wait for each
other. Each time a task reaches the barrier, the semaphore value is
decremented by one. If the result is greater than zero the task will
stall. If the result is zero or less, all tasks that were waiting at
this barrier are released.

**SEM** is the number of the semaphore that should control the barrier.

**VAL** is the new value for the semaphore, in the range -128 to +127.
If VAL is omited (recommended), the current semaphore value will be
decremented by one. Specifying VAL gives each task a chance to modify
the threashold value. A value of 1 or less always opens the barrier
(since the current task is already at the barrier, there is at least 1
task there...).

The best way to use barriers is to set the semaphore value with SETSEM
and have each task call BARIER without a VAL parameter. In this way, the
barrier just counts the tasks and opens when the preset number of tasks
has been reached. If you are planning to use the same barrier again,
remember to close it by calling SETSEM again.

BARIER can cause the error "number too big" if either SEM or VAL are too
high.

####  Setsem

    950 CALL LINK("SETSEM",SEM,VAL)

This subroutine lets you manually assign a value to a semaphore. It's
especially usefull to initialize a barrier, for instance.

**SEM** is the semaphore number. Cf GRAB.

**VAL** is the value you want this semaphore to take. It should be in
the range -128 to +127.

A "bad argument" error is issued if VAL or SEM is not an integer.

A "number too big" error occurs if you pick an illegal number for SEM.

###  Critical sections and cooperative multitasking

By default, INITSK enters preemptive multitasking. But you can use
CRITIC and ENDCRI to bracket sections in your program that should not be
interrupted when using preemptive multitasking.

While in a critical section, the only way you can switch task is to call
the scheduler. This can be done with routines like GRAB and WAIT, but
also with the dedicated YIELD routine. Note that yielding automatically
terminates the critical section.

####  Critic

    720 CALL LINK("CRITIC")

Enters a critical section. Interrupts still occur, but the scheduler
won't switch tasks any more.

####  Endcri

    880 CALL LINK("ENDCRI")

Leave critical section. Note that CRITIC is not additive: a single
ENDCRI enables task switching, no matter how many times CRITIC was
called before. And conversely: a single CRITIC defines a critical
section, even after several ENDCRI. In other words, it's a on/off
mechanism.

####  Yield

    800 CALL LINK("YIELD")

Returns the control to the scheduler. If a task switch can occur, it
will. Note that in this case, there is no need for the scheduler to
first jump to TSKINI: it can switch directly to wherever the next task
is waiting.

With cooperative multitasking, it is imperative that each and every task
frequently calls YIELD. If a task does not, it may stall the whole
program and multitasking won't be achieved effectively. This is why
preemptive multitasking is generally prefered.

With preemptive multitasking, calling YIELD is optional, but recommended
any time a task is idle (e.g. waiting for human input). Remember that
YIELD automatically terminates any pending critical section.

###  Instances and local variables

In Basic or Extended Basic programs all variables are accessible to all
parts of the program, i.e. there are no local variables as in C or
Pascal. This means that you have to be really carefull, because a given
variable used in the current task may be altered by another task, if
switching occurs. For instance, consider this loop:

    500 FOR I=1 to 100
    510 PRINT I
    520 PRINT A$(I)
    530 NEXT I

It should run fine, right? But suppose there is another task that also
uses the variable I. Now if the scheduler switches to this task while in
the loop, the value of I will be all messed up!

The conclusion is that each task should have its own set of variables:
avoid names like I, A\$, etc. My suggestion would be to use a mnemonic
trick, such as appending a different number at the end of each variable
belonging to a given task: I1 and A1\$ for one task, I2 and A2\$ for
another, etc.

This strategy is unfortunately impossible when two tasks share the same
section of your program, such as the above loop. This may occur because
they are performing identical functions (e.g. animate the many alien
ships that keep attacking the human player), or because they call a
common subprogram.

It is said that each task constitutes an "instance" of this routine or
subprogram. As all instances share the same code, they will use the same
variables. You must therefore save these variables when a task switches
off, and restore them when the task switches on. This is automatically
achieved by the same mechanism that saves the value stack. The only
thing you have to do is to tell the scheduler which variables you want
to save for which task. You can do that by including one or more calls
to LOCAL at the beginning of a task.

####  Local

    200 CALL LINK("LOCAL",I,A$,X2)

In this example A\$, I and X2 are three variables that we are declaring
as local. If necessary, you could have several calls to LOCAL, and
define as many local variables as you wish (although this would slow
down switching).

It is very important that any call to LOCAL occurs at the beginning of
the task, before any of the local variables is actually used. A good
practice is to FORK a new task beginning with its first LOCAL line.

LOCAL can cause a "memory full" error if the scheduler runs out of
memory. Remember that all active tasks, whether stalled or not, require
memory space to save all their local variables (and the Basic value
stack). For your information, each variable uses up 10 bytes.

A call to LOCAL has different effects depending on whether it deals with
a string or a numeric variable. For numeric variables, the current task
makes a copy of the current value of this variable and restores it every
time it is switched on. If a task did not declare this variable as local
(such as the parent task) it will still be able to use it, but at the
risk of seeing the value briskly altered, if a task switch has occured.
Only tasks that have declared a numeric variable as local can safely
assume that its value will never change unexpectedly.

For string variables, the current task grabs the string for its own use
and replaces it with an empty string. Which means that if you are
FORKing multiple instances, the parent task must reinitialize the string
every time! Every tasks that did not declare this string variable as
local share a "global" copy (initially empty) that may be modified by
those tasks that have their own local copies, when they switch between
each others.

This sounds a bit complicated, but you can make your life easier by
adopting a simple rule: Never use a local variable in a task that did
not declare it as local!

The exception to this rule is when the parent task needs to pass some
initial parameter to the child task. In this case the variable should be
initialize just before the FORK (possibly using a critical section for
numeric variables, when forking more than one task that uses this
variable).

------------------------------------------------------------------------

Multitasking assembly language

The following files are provided:

    Filename   Type  Contents
    TASKS/O    DF80  Scheduler. Allows multitasking in assembly language
    SUBS/O     DF80  Standard assembly utilities, adapted for multitasking
    TASKS/EQU  DV80  Equates to be used in your programs (on GPL disk)
    DEMO1/S    DV80  Tutorial. Basics.
    DEMO2/S    DV80  Tutorial. Priority.
    DEMO3/S    DV80  Tutorial. Mutex semaphores.
    DEMO4/S    DV80  Tutorial. General semaphores.
    DEMO5/S    DV80  Tutorial. Barriers.
    DEMO6/S    DV80  Tutorial. Context saving.
    DEMO7/S    DV80  Tutorial. Instance basics.
    DEMO8/S    DV80  Tutorial. Local variables.
    DEMO9/S    DV80  Tutorial. Full instanciation.
    DEMO10/S   DV80  Tutorial. Switch-time routines
    DEMO11/S   DV80  Tutorial. Critical sections. Standard utilities
    DEMO12/S   DV80  Tutorial. Real example: key scan with type-ahead buffer.

###  Loading the scheduler

The scheduler is a DF80 relocatable tagged-object file named TASKS/O. It
should be loaded along with your program, with Editor/Assembler option
3, or an equivalent loader (such as Fweb). The current version is a
little short of \>800 bytes in size.

The file defines the following routines:

    INITSK  Initialize multitasking
    FORK    Create a new task
    KILL    Kill a task
    DIE     Kill the current task
    GRAB    Grab a semaphore
    WAIT    Wait for a semaphore
    BARIER  Wait at a barrier
    DROP    Drop a semaphore
    RAISE   Raise a semaphore
    YIELD   Yield to another task
    GETTCB  Get a pointer to a task control block

And the following data words:

    CRITIC  Flag for critical sections
    TSKERR  Error code
    ERRSEM  Semaphore for fatal error barriers
    VADDR   Current VDP address
    VR0     Copy of VDP registers (8 bytes)

###  Initializing multitasking

Just call the INITSK routine. It clears the task list, hooks the
interrupt routine and returns with an implicit LIMI 2 to enable
interrupts. If you want cooperative multitasking, add a LIMI 0 just
after the call to INITSK.

    BLWP @INITSK      or      BLWP @INITSK      or      BLWP  @INITSK
    DATA tcbptr               DATA >000r                DATA   >FFFF

**Tcbptr** is the address of the Task Control Block (TCB) to be used for
the main program. Alternatively, the data word can contain a number "r"
from 0 to 15 (i.e. \>000F), which indicates in which register the
address of the TCB is to be found.

If the data word or the register content is \>FFFF, no TCB is specified.
The scheduler will provide a minimal TCB of its own, with a priority of
\>60, no context saving, and no local memory.

#### Technical note

INITSK intalls an interrupt service routine hook at address \>83C4. It
you already had a hook in there, the scheduler will link to it. If a
task switch is due to occur, your routine will be called before the
switch takes place. You must return with `B *R11` for the switch to
occur. If you return with `LWPI >83C0` and `RTWP`, the current task will
remain active.

If you would like to remove, or modify your hook, its value can be found
12 bytes after VR0:

           LI   R1,VR0
           CLR  @12(R1)   to remove your hook

###  Creating a new task

The scheduler allows upto 32 tasks at any given time, including the main
program. To create a new task, set up a Task Control Block (see below),
then call the FORK routine.

    BLWP @FORK        or        BLWP @FORK
    DATA tcbptr                 DATA >000r

**Tcbptr** is the address of the Task Control Block (TCB) to use for the
newly created task. Alternatively, this address can be in a workspace
register whose number is passed in the data word (the address cannot be
\>FFFF with FORK).

The structure of a minimal TCB is the following:

    TCB1  BYTE 0         priority (0 = same a parent task)
          BYTE 0         internal use
          DATA >0000     flags (0 = minimal TCB)
          DATA 0         workspace (0 = same as parent task)
          DATA 0         start point (0 = after the FORK statement)
          DATA 0         internal use (status buffer)
          DATA 0         internal use (timer count)
          DATA 0         internal use (semaphore ptr)

As you see, it's very simple: it basically consists in a string of 14
null bytes!

If you wanted to, you could specify a starting address for the child
task, in the 4th word of the TCB. If you leave this word as 0, the child
task will start where the parent task returns, i.e. after the call to
FORK. You can differentiate them easily, because the parent task return
with the Eq bit reset, whereas the child task starts with the Eq bit
set. So you can use `JNE `to trap the parent task, or `JEQ `for the
child task.

The parent task returns with the ID of the newly created (child) task in
R0. Hint: you may want to save this value for further use... If this
value is zero, an error occured and no task was created. The error
number can be found in the word TSKERR.

###  Terminating a task

You can terminate tasks by calling either the KILL routine, or the DIE
routine.

#### Kill

    BLWP @KILL

Terminates the task whose ID is in R0 (I told you to save this value).
If R0 contains zero, the current task will be terminated, which is
equivalent to a call to DIE.

If an invalid task ID was placed in R0 the routine returns with the Eq
bit set, and TSKERR will contain the value RNOTID.

####  Die

    BLWP @DIE

Terminates the current task

Caution: if you terminate all tasks in a program, the scheduler can't
run anything and will reset the TI-99/4A. The same thing will happen if
all tasks are hanged by semaphores.

###  Terminating multitasking

To terminate multitasking operations, you can call ENDTSK

    BLWP @ENDTSK

ENDTSK unhooks the interrupts and clears the list of tasks. It also
places -1 in TSKERR. Then it calls the switch-time routines (if any) for
each and every task (whether it is ready to run or hanged), including
the current one. Finally ENDTSK returns to your program, that will not
be considered as a task any more.

You can re-initiate multitasking by calling TSKINI again. Until you do
that, do not call any other routine in the scheduler, as they will not
function properly.

###  Setting priorities

You can affect the way a task is scheduled by changing the priority
value in its TCB. Note that, for the current task, the change will only
become effective after a task switch has occured.

There are four priority classes:

- Low priority : \>01 to \>3F
- Average priority: \>40 to \>7F
- High priority : \>80 to \>BF
- Critical task : \>C0 to \>FF

The scheduler will never select a task if a task in a higher priority
class can be run instead.

Within a given class, the scheduler will run each task in turn at a
frequency determined by the respective task priorities. For instance, a
task with a priority of \>20 will run twice as often as a task with a
priority of \>10, but twice less often than a task with a priority of
\>3F. The same is true for priorities \>60 and \>50 in the "average
priority" class: within the class, a priority of \>60 - \>40 = \>20 is
twice higher as a priority of \>50 - \>40 = \>10.

###  Synchronizing tasks

Task synchronisation can be achieved with semaphores. A semaphore is
nothing more than a data word that should be part of your program's data
and initialized to an appropriate value (generally zero). If it contains
a positive value, it has the potential to suspend (hang) a task. A null
or negative value will let the task run.

Three routines have the potential to hang a task, i.e. not to return
until a semaphore reaches a given value: WAIT, GRAB and BARIER. (Yes I
know, barrier is spelled with two Rs, but label names are limited to 6
characters).

You can manipulate the value of a semaphore with the routines DROP and
RAISE (although BARIER and GRAB also change the semaphore value). You
can also change it directly, but this will not free any currently hanged
task!

####  Wait

    BLWP @WAIT         or        BLWP @WAIT
    DATA semptr                  DATA >000r

**Semptr** is the address of a semaphore, and can also be specified in
register r.

WAIT tests the value of the semaphore word. If it is 0 or negative
(\>8000-FFFF) it returns immediately, with the Eq bit reset. If the
semaphore value is positive(\>0001-7FFF), the task hangs and will not
resume until the semaphore value is brought back to 0 or less by the
DROP routine. When the task is freed, the routine returns with the Eq
bit set.

####  Drop

    BLWP @DROP         or       BLWP @DROP
    DATA semptr                 DATA >000r

**Semptr** is the address of a semaphore. This address can also be
placed in register r, if you use the second syntax.

DROP decreases the value of the specifed semaphore by one. If the result
is zero or less, it looks for a task hanged by this semaphore (due to a
call to WAIT, GRAB or BARIER), with a priority class higher or equal to
that of the current task. If no such task is found, execution continues
with the current task and the Eq bit will be reset.

If a hanged task was found, the outcome depends on the routine that
suspended it:

- If it was GRAB, the task is switched on and the semaphore is
  incremented by one again.
- If it was WAIT, the task is switched on, but the semaphore is left
  unchanged.
- If it was BARIER, the scheduler frees the task and attempts to find
  another one. It's only when no more tasks can be freed that the
  scheduler switches one on.

If a task switch occured, the Eq bit will be set when DROP finally
returns to the calling task.

####  Raise

    BLWP @RAISE       or        BLWP @RAISE
    DATA semptr                 DATA >000r

**Semptr** is the address of a semaphore. This address can also be
placed in register r.

RAISE increments by one the value of the specified semaphore. If the
result is zero or less, it searches for a task hanged by this semaphore
and switches it on. If no such task exists (or if it is in a lower
priority class than the current task), the current task continues with
the Eq bit reset. If RAISE caused a task switch it will return with the
Eq bit will be set.

####  Grab

    BLWP @GRAB         or        BLWP @GRAB
    DATA semptr                  DATA >000r

**Semptr** is the address of a semaphore. This address can also be
placed in register r.

GRAB is a combination of WAIT and RAISE. It first checks the semaphore
value. If it is positive the task hangs until the semaphore becomes 0 or
less. When it's the case, the task increments the semaphore value by one
and returns with the Eq bit set. If GRAB is able to return right away,
the Eq bit will be reset.

This is safer than calling WAIT and RAISE in succession: there is always
a possibility that a task switch occurs just after WAIT, in which case
the semaphore value may become positive even though the task went
through WAIT without hanging!

GRAB is usefull to implement a mutex, or mutually exclusive semaphore.
Imagine a resource that can only be used by one task at a time. A
printer is a good example: you wouldn't want two tasks to take turns
printing one line at a time, would you?

So each task should first grab a semaphore, then print what it needs to
print, and drop the semaphore once done. If another task tries to print
in the mean time, it will hang at the GRAB statement until the first
task has released the semaphore. Then the second task grabs the
semaphore and gains exclusive control of the printer, etc.

####  Barrier

    BLWP @BARIER        or        BLWP @BARIER
    DATA semptr                   DATA >000r

**Semptr** is the address of a semaphore. This address can also be
placed in register r.

A barrier is a special type of semaphore: it suspends any task that
reaches it, until a preset number of tasks has reached the barrier. At
this point the barrier is opened and all tasks are free to go.

To implement a barrier, use a semaphore with an initial value
corresponding to the number of tasks that should reach the barrier
before it opens. Then have each task call BARIER: it decrements the
semaphore value by one. If the result is not zero yet, this task hangs.
If the result is zero (or less), BARIER frees all other tasks hanged by
this barrier and returns to the caller.

The task that caused the barrier to open returns from BARIER with the Eq
bit reset. All other tasks return with the Eq bit set.

Note that the semaphore will need to be initialized again before the
barrier can be reused!

###  Critical sections and cooperative multitasking

There may be cases when you don't want your program to be interrupted:
time critical processes, routines that are called by different tasks but
cannot manage two tasks at a time, etc. These so-called critical
sections can be created by bracketing them with a `SETO @CRITIC` and a
`CLR @CRITIC`.

CRITIC is a flag that is part of the scheduler and DEFined inside the
TASKS/O file. When CRITIC is zero, multitasking occurs normally. When
CRITIC is \>FFFF, interrupts still occur, but the scheduler won't switch
tasks.

Alternatively, you could use `LIMI 0` and `LIMI 2` to bracket critical
sections, in which case interrupts will not occur. The two solutions are
not completely equivalent however. Remember that the console interrupt
routine handles sprites in auto-motion, automatic sound playing, the
\ key and the screen-saving timeout. Also, peripheral card
interrupts won't be answered if you use `LIMI 0`. Therefore, it is
generally better to use `SETO @CRITIC`, unless you know for sure that
interrupts can safely be disabled in a given section of your program.

By default, INITSK enables the interrupts so multitasking will be
preemptive. If you prefer cooperative multitasking, you can either place
a `LIMI 2` just after the call to INITSK so as to disable interrupts,
use a `SETO @CRITIC` to prevent task switching upon interrupts, or both.

Regardless of the method chosen, it is critical that each task
periodically returns control to the scheduler, so that other tasks get a
chance to run. This can be achieved by calling the YIELD subroutine,
although most of the other routines will also seize the opportunity to
switch tasks.

####  Yield

    BLWP @YIELD

Relinquish control to the scheduler. If a task switch can occur, it
will. A task should always call YIELD when it is idling. For instance,
suppose you scanned the keyboard and no key was pressed. Given the slow
human reaction time it is likely that, if you rescan the keyboard right
away, there still won't be any key pressed. So now is a good time to
yield. When YIELD returns, scan the keyboard again, etc.

Another way to switch tasks in a cooperative program, is to temporarily
enable preemptive multitasking, with a `LIMI 2, LIMI 0` pair of
instructions for instance. Toggling CRITIC also works, but in a slightly
different manner: if a task switch does occur immediately, the new task
will run in preemptive conditions until a `SETO @CRITIC` is encountered.

###  Context switching

It is possible for a task to save several parameters as it switches off,
and to retrieve them when it's switched on again. Currently, the
scheduler handles the following parameters:

- GROM base, as specified in word \>837A in the scratch-pad. This is the
  address of the GROM port.
- Current GROM address, as obtained from the GROMs.
- VDP address. The address can always be set when switching on, it can
  only be saved (when switching off) if all VDP operations were
  performed using my VDP access subroutines. These routines save the
  current address into the scheduler variable VADDR (initially, it is
  \>0000).
- VDP registers. These can always be set when switching on. They can
  only be saved reliably if my VWTRX routine was used to set the VDP
  registers: it saves them into the scheduler data area, in eight bytes
  starting at address VR0. The scheduler also updates the copy of VDP
  register 1 in the scratch-pad (byte \>83D4, used by the keyboard
  scanning routine). If VWTRX has not been used yet, the default values
  are those of the Editor/Assembler module: \>00, \>E0, \>00, \>0E,
  \>01, \>06,\>01, \>F5.

To enable saving/retrieval of a given parameter, you must set the
corresponding flag in the TCB. You must also provide space in the TCB to
store this variable. Remember that a task is always switched on before
it is switched off, so make sure you initialize these variables with
some meaningfull value. Alternatively, you can set a value as \>FFFF,
and FORK will automaticatically initialize it with the current value.

Flags:

    >8000: Save / retrieve GROM base (from/to >83FA)
    >4000: Save / set GROM address (from/to GROMs)
    >2000: Set VDP address
    >1000: Save VDP address (from VADDR)
    >0800: Set VDP registers
    >0400: Save VDP registers (from VR0 and following bytes)

Structure of a TCB with context saving:

    TCB1   BYTE 0            priority
           BYTE 0            internal use
           DATA >FC00    <== flags
           DATA 0            internal use
           DATA 0              "
           DATA 0              "
           DATA 0              "
           DATA 0              "
           DATA >9804        GRAM base buffer (set it)
           DATA >FFFF        GRAM address (use current)
           DATA >1234        VDP address (set it)
           BYTE >FF,>FF,>00,>0E,>01,>06,>00,>F5  VDP registers (use current)
           DATA 0            switch-time routine (none)

###  Switch-time routine

If you need to save other parameters, or to perform any kind of
operation at switching time, you can place the address of a switch-time
routine in the TCB, and set flag \>0010 to indicate its presence.

The scheduler will call your switch-time routine with a BLWP in six
distinct occasions. You can distinguish them by testing the value found
\*R14+ (You MUST fetch this value, so as to skip a word when returning
with RTWP).

- When the task is first created, but is not running yet. \*R14 = \>8000
- When the task has been selected for running, before the context was
  loaded. \*R14 = \>FFFF
- When the task is about to run, after context was loaded (if any).
  \*R14 = \>FFFE
- When the task is due to switch off, before context was saved. \*R14 =
  \>0000
- When the task is about to switch off, after context was saved. \*R14 =
  \>0001
- When the task is killed, or multitasking is terminated. \*R14 = \>8001

If several tasks use the same switch-time routine, you can get a pointer
to the task's TCB from the word @4(R13). Do not alter this word, nor any
other in the scheduler's workspace!

An example of need for a switch-time routine would be a memory extension
card that does bank switching. You could use such a routine to select
the banks required by a given task.

Make sure you switch-time routine runs fast, as it may tremendously slow
down program execution. Remember that tasks are switched as often as 60
times per second (50 on European consoles).

#### Technical note

If necessary, you could abort the switching operation by redirecting R14
to a `RTWP `instruction. This causes the scheduler switching routine to
return before switching was completed, so the same task will keep
running (however, some context manipulation may already have been
performed...). Don't do this during task creation though, as no
switching will occur anyhow: the creation call is only here to give you
an opportunity for some initialization. Don't do it during termination
either, except if TSKERR is -1. In this case redirecting R14 to a
`RTWP `returns from ENDTSK immediately, without calling the switch-time
routines for the remaining tasks, nor clearing the rest of the task
list.

###  Instances and local memory

It may occur that a given portion of your program is run simultaneously
in several tasks. This "instanciation" (what an hugly neologism) process
has a main drawback though: since tasks share the same lines of program,
they act on the same variables and they'll overwrite each other's data!
To some extent, this can be prevented by placing all data in registers
and assigning a different workspace to each task. But that's only 16
variables...

A way to extend this mechanism is to reserve a different memory area for
each task (known as a local memory), and have a workspace register point
at it. All variables can then be accessed as offsets from this pointer.
Upon request, the scheduler will conveniently place the proper address
in the register of your choice each time a task is switched on.

Example:

    Without instanciation       With instanciation
                                THIS   EQU   0
                                THAT   EQU   2

           CLR  @THIS                  CLR    @THIS(R8)
           SETO @THAT                  SETO   @THAT(R8)
    *                           *
    THIS   DATA >1234           LOCAL  DATA  >1234   <--- R8 points here
    THAT   DATA >5678                  DATA  >5678
                                ENDLOC

Actually, the address of your local data area does not need to end up
into a register: it could be placed anywhere in memory. A register is
just more convenient. Similarly, you can cause the scheduler to place
the address of your workspace into a register or at any given address.

All this is done by setting the proper flags, and including the required
addresses in the TCB.

Flags:

    >0020: Place a pointer to the local data area into a register/address
    >0040: Place a pointer to the workspace into a register/address
    >0080: Automatically assign a data area when creating a new instance

Structure of a TCB for instanciation:

    TCB1   BYTE 0            priority
           BYTE 0            internal use
           DATA >00E0    <== flags
           DATA 0            internal use
           DATA 0                "
           DATA 0                "
           DATA 0                "
           DATA 0                "
           DATA 0            local context
           DATA 0                "
           DATA 0                "
           BYTE 0,0,0,0,0,0,0,0  "
           DATA 0                "
           DATA LOCAL        address of local data area
           DATA ENDLOC-LOCAL size of local data area
           DATA >0008        where to place ptr to data area (here: in R8)
           DATA THERE        where to place workspace ptr (here: in data word THERE)

You can even have the scheduler assign a local data area to each task,
as it creates it. This way, you can use the same TCB for each task
instance. There are two requirements though:

- The TCB must be part of the data area.
- There must be enough free space just after the data area to create a
  copy of it for each task.

If you set the \>0080 flag in the TCB, the FORK function will copy the
whole data area somewhere in the memory immediately downstream of it.
That's why you should reserve enough space for as many times the data
area size, as you plan to have instances. The TCB must be part of the
local area, so that each task can have its own TCB. You may also want to
have the task's workspace in the local area, unless it's ok for all
instances to share the same workspace. Personally, I like to place my
workspace at the beginning of the data area: this way I can use the same
pointer for both.

    * Example of local data area
    WREGS  BSS  32              workspace

    THIS   DATA >1234           some data
    THAT   DATA >1234

    TSK1   BYTE 0,0             TCB
           DATA >00A0
           DATA WREGS
           DATA 0,0,0,0,0,0,0,0,0,0,0
           DATA WREGS
           DATA ENDLOC-WREGS
           DATA >0007
           DATA 0

    COUNT  DATA 123            more data used by the task
    DSR    TEXT 'DSK1.'
           EVEN                *must* end on a even address
    ENDLOC

           BSS  ENDLOC-WREGS   space for 1 instance
           BSS  ENDLOC-WREGS   space for 2 instances
           BSS  ENDLOC-WREGS   etc.

###  Fooling around with TCBs

The Task Control Block is the key structure that governs the behaviour
of a task. In addition, it is used by the scheduler to store important
variables upon task switching. Therefore, TCBs should always be modified
with due caution. In particular, the scheduler does not have time for
sanity checks, so absurd values (that would be rejected by FORK) will be
blindly accepted upon task switching. Also remember that the TCB of the
current task is different from these of currently "off" tasks: part of
its contents will be overwritten when the task switches off.

Finally, beware that a task switch may occur in the middle of your
modifications. You should thus make sure that the task you are modifying
will not be selected before you are done. This can be achieved by
INVerting its semaphore word (at offset +12 whithin the TCB), but don't
forget to `INV `it again when done otherwise your task will never run!

####  GetTCB

The routine GETTCB can be used to obtain the address of a TCB, by
providing the task ID. Don't assume that you know where the TCB is: if
the task is instanciated, each instance will have its own copy of the
TCB, somewhere downstream in the local memory area...

    BLWP @GETTCB

Called with the task ID in R0 (or \>0000 for the current task), returns
a pointer to the TCB in R1. If an error occurs (e.g. the task ID is
incorrect) GETTCB returns with the Eq bit set, and R1 contains zero.

If you call GETTCB with \>FFFF in R0, it returns a pointer to the task
list in R1. You can the walk the list with a series of `MOV *R1+`, and
so get pointers to the TCBs of all tasks currently alive (whether they
are ready to run or not), in no particular order.

Possible values for each word in the list are:

- \>0000: Empty slot, skip this word
- \>0001: End of list
- Other: Pointer to a TCB

####  Structure of a Task Control Block

Now let's examine the different fields that make up a TCB. The names
correspond to labels defined in the TASKS/EQU file. Except that in the
file they are provided as EQU offsets, so that you can use them with a
task pointer. For instance: `CLR @TVAD(R1)`.

    TPRIOR BYTE 0            priority
    tbyte1 BYTE 0            internal use only (no EQU)
    TFLAGS DATA >0000        flags
    TWS    DATA 0            workspace buffer
    TPC    DATA 0            program counter buffer
    TST    DATA 0            status buffer
    TCOUNT DATA 0            time counter
    TSEM   DATA 0            semaphore ptr
    *-------------optional from here down----------------*
    TGBA   DATA 0            GRAM base buffer
    TGAD   DATA 0            GRAM address
    TVAD   DATA 0            VDP address
    TVR    BYTE 0,0,0,0,0,0,0,0  VDP registers
    TSWI   DATA 0            switch-time routine
    TLOCAL DATA 0            address of local data area
    TLSIZE DATA 0            size of local data area
    TLPTR  DATA 0            were to place ptr to local data area
    TWSPTR DATA 0            where to place workspace ptr

**TPRIOR** Priority. The meaning of this field
has been discussed [above](#set%20priority). If you modify it "on the
fly", it will take effect immediately and is likely to affect the next
task switch. Note that only FORK understands a priority of zero as "same
as parent"

**TBYTE1**. This byte is reserved by the
scheduler for storage of some status flags (currently: whether the taks
is hanged by a GRAB, a WAIT, or a BARIER). Better don't touch this
field. Always use byte-oriented operations to deal with the priority
byte.

**TFLAGS**. Here is a summary of the flags
currently understood by the scheduler.

    >8000: Save / set GROM base (from/to >83FA)
    >4000: Save / set GROM address (from/to GROMs)
    >2000: Set VDP address
    >1000: Save VDP address (obtained from VADDR)
    >0800: Set VDP registers
    >0400: Save VDP registers (obtained from VR0 and following bytes).
    >0080: Automatically assign a data area when creating a new instance
    >0040: Place a pointer to the workspace into a register/address
    >0020: Place a pointer to the local data area into a register/address
    >0010: Call a switch-time routine

If you change a flag, it will only have effect the next time this task
is switched on or off. Be carefull that many flags affect the size of
the TCB. The length of the TCB is defined by the last word for which a
flag is set (even if all previous words are not used, they are still
there). Hence it is useless (and risky) to enable context saving in a
minimal TCB that does not contain room for this purpose!
**TWS** Task workspace buffer. Note that only FORK
recognises a zero as "same as parent". Any value that you put in here
will be used as workspace, no matter how crazy it is, the next time this
task is switched on.

For the current task, this value is the worskpace used when the task was
switched on. There is no guaranty that it is still the current
workspace. This value will be updated when the task switches off.
**TPC** Task PC buffer, indicates where execution
will resumes next time the task is switched on. Note that only FORK
understands a zero as "after the FORK statement". Any value placed in
here will be used as a start point next time the task switches on.

For the current task, this value indicates where the task resumed last
time it was selected. It will be overwritten when the task switches off.
**TST** Task status buffer. This value will be
placed in the status register when the task is switched on. Caution: it
contains the interrupt mask! If you modify it, make sure it ends with
\>xxx2 for preemptive multitasking, with \>xxx0 for cooperative
multitasking. For the current task, this value will be overwritten when
the task switches off.
**TCOUNT** Time counter. This variable is
initialized by FORK and used by the scheduler to determine which task
should run next. The value will be reset as zero for the selected task.
For all other tasks that could have been selected but were not (i.e. not
hanged, same priority class), this word will be incremented by the
in-class priority value (after masking out the class bits).

Setting this word as \>FFFF almost guaranties that the task will run
soon, provided it has the right priority class and is not stalled at a
semaphore. For the current task, this word is always zero, any change
will become effective when the task is switched off.

Conversely, setting TCOUNT as \>0000 renders the task less likely to
run. It may still be selectect though, if no other task is ready to run.
**TSEM** Semaphore buffer. This variable is used
by the scheduler to store a pointer to the semaphore currently hanging
the task. This word must be \>0000 for the task to run. You can clear a
non-zero word, to make the task ready to run, or use a non-zero value to
prevent a task from running.

The latter is a bit dangerous: make sure you don't overwrite an existing
semaphore pointer! I recommend using the INV instruction: it will turn
\>0000 to \>FFFF and modify any existing semaphore value so that it will
not be cleared by a DROP in another task. A second INV will restore the
previous value if needed. A potential problem is that the actual
semaphore may have been cleared in the mean time, and our task won't
know about it. Thus it will still hang, as the scheduler has no time to
check semaphore values (the mere presence of a semaphore pointer in the
TCB marks the task as suspended).
The remaining fields are only meaningfull if the corresponding flag is
set in TFLAGS. They may be missing in some TCBs, but their order is
invariable (i.e. TCBs "grow" from the bottom).
**TGBA** Buffer for GROM base. This value will be
transfered to word \>83FA when the task switches on, provided the
corresponding flag is set in TFLAG. Caution: a value that does not
correspond to a GROM base (i.e. in the range \>9800-983C) will crash the
GPL interpreter and any program making use of this value.

The current task will overwrite this value with the content of word
\>83FA as the task switches off.
**TGAD** Buffer for GROM address. This value will
be set as the current GROM (or GRAM) address when the task is switched
on, provided the corresponding flag is set in TFLAGS. The current task
overwrites this value with the current GROM address, as obtained from
the GROMs (with the -1 address correction).
**TVAD** Buffer for VDP address. This value will
be set as the current VDP address when the task switches on, if the
corresponding bit is set in TFLAGS. It must contain the read/write
indication bit, i.e. write operations add \>4000 to the actual address.
The value is also copied into VADDR.

The current task will overwrite this value with the content of word
VADDR in the scheduler memory, if the corresponding (and distinct) bit
in TFLAGS is set.VADDR is maintained by the VDP access subroutines
VSBRX, VMBRX, VXBWX, VMBWX and VIBW.
**TVR** Buffer for the eight VDP registers. If the
corresponding bit is set in TFLAGS, these 8 bytes will be send to the 8
VDP registers when the task is switched on. They will also be copied
into byte VR0 and following in the scheduler's memory.

Another bit in TFLAGS causes these values to be overwriten by byte VR0
and following, as the current task switches off. VR0 and the next 7
bytes contain copies of the VDP register values set by the VWTRX
subroutine.
**TSWI** Address of the switch-time routine. The
use of this routine has been discussed [above](#switch-time). It must be
enabled by setting the corresponding bit in TFLAGS. Once this is done,
the routine will be called twice when the task is switched on, and twice
when it switches off. (Remember that the current task is next due to
switch off, not on).
**TLOCAL** Address of the local memory area. This
address is used by TFORK to create instances, provided the corresponding
bit is set in TFLAGS. When a task is switched on, this value will be
placed at the address specified in TLPTR, if the corresponding bit is
set in TFLAGS. Nothing happens when the task is switched off.
**TLSIZE** Size of the local memory area. This
value is only used by TFORK to create instances. Modifying it on the fly
has no effects.
**TLPTR** Pointer to local memory area. This can
be either a register number, from 0 to 15, or an address. If the
corresponding bit is set in TFLAGS, the address of the local memory area
will be placed in the specified register or address when the task is
switched on. Nothing happens when the task is switched off.
**TWSPTR** Pointer to the workspace. Can be an
address or a register number, from 0 to 15. If the corresponding bit is
set in TFLAGS, the address of the task workspace will be placed in this
register, or at this address, when the task switches on. Nothing happens
when it switches off.

###  Scheduler error codes

When an error occurs, an error code can be retrieve from the word
TSKERR. If no error occured, the content of this word is meaningless.
Currently, the following codes are defined (in the TASKS/EQU files).

    RMEM    EQU     >0001   Out of memory. Most likely, too many tasks were created.
    RINST1  EQU     >0002   Instanciation error #1. The TCB is not in local memory.
    RNOTID  EQU     >0003   Wrong task ID (may have been valid, but task is now dead).
    RNOTPT  EQU     >0004   Not a valid address for a pointer (e.g. ROM space).
    RGPL    EQU     >0005   Error in TCB for GPL task (most likely, FGAD flag missing).
    RNORUN  EQU     >0081   Fatal error. Nothing to run.

When checking the error code, use the label rather than the numeric
value. This way, if the code change in the future, your program will
still work (you will only need to update the /EQU file).

####  Fatal errors

If the scheduler cannot find a task ready to run, either because you
killed them all, or because they are all hanged at semaphores, the
TI-99/4A will be reset.

This kind of error typically occurs when task 1 waits for a semaphore
grabbed by task 2, and then task 2 starts waiting for a semaphore
grabbed by task 1... One way to avoid this error is to number your
semaphores (SEM1, SEM2, etc) and adopt the rule that a task should never
wait for a semaphore with a number smaller than the highest one it has
already grabbed, but not dropped yet.

Of course, there is also the case when a given task is killed before it
has dropped all the semaphores it had grabed.... Or when a tasks grabs
twice the same semaphore... There is no failsafe strategy here, apart
from carefull programming.

The scheduler provides you with a way to catch fatal errors before it
resets the TI-99/4A. To this end, it defines the semaphore ERRSEM, with
an initial value of \>3000. You can have your error-handling task wait
for that semaphore with either GRAB, WAIT, or BARIER (the latter is
recommended). When a fatal error occurs, the scheduler clears the
semaphore and attempts to run any task(s) that were waiting for it.

A task that handles the error successfully should reset the semaphore as
\>3000, a task the cannot handle it should simply INCT the semaphore and
loop back to the BARIER call. If no tasks was able to handle the
lock-up, the scheduler will reset the TI-99/4A (the scheduler won't free
the semaphore again if its value is less than \>2000, so as not to loop
forever).

###  Standard assembly utilities

For your convenience, the scheduler comes with another file, SUBS/O,
which contains the standard assembly language utilities, suitably
modified for multitasking (namely, they are enclosed in a critical
section). While I was at it, I also added a few extra features.

**VSBRX** Just like VSBR, but tests for 0 in R2 and saves the final
address in VADDR.

**VMBRX** Just like VMBR, but tests for 0 in R2 and saves the final
address in VADDR.

**VSBWX** Just like VSBW, but tests for 0 in R2 and saves the final
address in VADDR.

**VMBWX** Just like VMBW, but tests for 0 in R2 and saves the final
address in VADDR.

**VIBW**. New routine: VDP Identical Bytes Write. Writes multiple
occurences of the same byte in VDP memory and saves the final address in
VADDR.
Parameters:
R0 = VDP address
R1(msb) = byte to write
R2 = number of repeats (\>0000 does nothing).

**VWTRX** Just like VWTR, but saves register values in VR0 and following
bytes in the scheduler memory.

**KSCANX** Just like KSCAN, but returns with the Eq bit set if a new key
was pressed, and with the H bit set if the same key was held.

Usage:

          BLWP @KSCANX
          JEQ  NEW
          JH   SAME
          JMP  NONE

**XMLLNX** Just like XMLLNK.

**GPLLNX** Just like GPLLNK, but does not require the Editor/Assembler
cartridge. It makes use of some data byte in the console GROMs to
simulate a dummy XML and regain control when then called GPL routine
returns. As this may be console dependent, the critical values can be
modified by changing the two words immediately preceding GPLLNX:

           LI   R1,GPLLNX    get ptr to GPLLNX
           MOV  ...,@-2(R1)  change the RAM vector
           MOV  ...,@-4(R1)  change the GROM address
The GROM address must be that of a \>0F byte, which will be interpreted
as an XML. The next byte will indicate where the XML vector is located.
This must be an address in RAM. [Calculate](../gpl2.htm#XML) its value
and place it in the word @-2(R1).

GPLLNX also provides an alternative syntax, to call routines that are
normally called with the GPL routine G@\>0010. This requires passing the
GROM base together with the desired address:

           BLWP @GPLLNK
           DATA >0000       Flag: alternate syntax
           DATA >98xx       GROM base
           DATA address     Address in GROM/GRAM memory
**DSRLNX** Just like DSRLNK, but the DATA statement is optional: if
omited it is assumed to be DATA \>0008, i.e. calling a DSR. Also, when
DSRLNX has found the DSR/subprogram, it saves all parameters for use by
DSRRPT.
**DSRCRU** New routine. Just like DSRLNK but begins scanning the
peripheral cards with the CRU base specified in word \>83D0.
**DSRRPT** New routine. Calls the same DSR or subprogram that was
previously called with DSRLNX. Caution: make sure a task switch did not
result in calling DSRLNX in another task. This is a good example of what
a mutex semaphore can be used for...

------------------------------------------------------------------------

Multitasking GPL programs

The scheduler for GPL is the same as for assembly, with a thin wraper
that allows you to call each routine with an XML. In order not to
uselessly clutter the XML table, you have the option to call all
routines with a single XML.

Remember that the GPL interpreter performs a `LIMI 2, LIMI 0` in between
instructions, except while it's in the FMT sub-interpreting mode (screen
access: always `LIMI 0`). Therefore, multitasking will always be
preemptive, unless you perform a `DST >FFFF,@CRITIC`. Assembly routines
called from GPL (with either a XML instruction or the DSR-calling
routine `G@>0010`) will be cooperative. If you want preemptive
multitasking in assembly, you must include a `LIMI 2` in the called
assembly routine.

The following files are provided:

    Filename     Type    Contents
    TASKGPL      Prog    Scheduler. Allows multitasking in assembly and GPL
    TASKG/EQU    DV80    Equates to be used in your programs
    GPLDEMO/T    DV80    Demo program (source text)
    GPLDEMO/C    DF80    Demo program (tagged-object code)
    GPLDEMO      Prog    Demo program (GROM image, in GramKracker format)
    TASKG/S      DV80    Source file: XML wrapper around the assembly scheduler

###  Loading the scheduler

The GPL scheduler comes in the form of a "program" file, called TASKGPL.
It is nothing more than the assembly scheduler (TASKS/O) with a thin
wrapper to allow XML calls. If you are interested, have a look at the
file TASKG/S, that contains the source for the wrapper.

The scheduler can easily be loaded from disk into the high memory
expansion with the following routine:

           MOVE 22,G@PAB,V@>1000          or whatever VDP address you like
           DST  >1009,@>8356              pointer to name size byte
           CALL G@>0010                   call DSR
           BYTE >08
           BS   ERROR                     handle errors

           DST  V@>1102,@>834A            get size
           DST  V@>1104,@>834C            get address (should be >A000)
           MOVE @>834A,V@>1106,@0(@>834C) copy program in CPU mem
           ...                            and use it

    PAB    DATA    >0500                  PAB for file access
           DATA   >1100                   VDP buffer address
           DATA    >0000
           DATA    >2000                  max file size
           BYTE    >00
           STRI    'DSK1.TASKGPL'          filename (string including size byte)

This will load the scheduler at the beginning of the high memory
expansion, from \>A000 to \>AA20. You can then use XML \>B1 through \>BB
to access the scheduler. Alternatively, you can access it exclusively
through XML \>B0 and use the other XML entries for your own vectors. In
case you would want to restore them afterwards, there is an internal
copy (used by XML \>B0) at addresses \>A020-A037.

You can access the scheduler variables at the following addresses
(included in the file TASKG/EQU):

    CRITIC  EQU     >A286                   >FFFF = critical section, >0000 = normal
    TSKERR  EQU     >A288                   scheduler error code
    ERRSEM  EQU     >A28A                   semaphore for fatal error barrier
    VADDR   EQU     >A28C                   VDP address buffer (not used by GPL)
    VR0     EQU     >A28E                   VDP register buffers (not used by GPL)

###  Context saving

The VDP context cannot be saved in GPL, because the GPL interpreter uses
its own routines rather than those I modified for use with multitasking.
It should not matter too much though, since GPL considers all VDP access
operations as critical sections. Just remember that you cannot assume
the VDP address will be maintained from one instruction to the next
(which should never be assumed with GPL anyhow).

The GROM context must always be part of the TCB, since this is the way
GPL will be multitasked. The XML wrapper around FORK will reject a TCB
that does not have the corresponding flag set. Saving and retrieving the
GROM base is optional.

The GPL interpreter uses some critical bytes in the scratch-pad memory
(\>8300-83FF), that must be saved upon task switching for proper
functionning. The GPL scheduler provides a default switch-time routine
that is automatically implemented if you don't specify one. This routine
saves and retrieves the following:

- Bytes \>8372-8373
- Bytes \>837C-837F
- Subroutine stack (\>8380 to pointer in \>8373)
- Data stack (\>83A0 to pointer in \>8372)

These values are saved at the top of your local memory area, provided
you specifed one (you don't need to set the "instanciation" flag). If
you didn't specify a local area, the default switch-time routine will
not be installed !

If you want to save more context, you will have to implement your own
switch-time routine in assembly. You can finish by linking to the
default routine with `B @>A040`: it will save the above parameters and
return to the scheduler with `RTWP`. Caution: the default routine
performs a `MOV *R14+` to get the calling code, so if your routine also
needs this code word, it should get it with `MOV *R14`, so that R14 does
not get incremented twice!

###  XML routines

There are 12 routines that can be called from GPL. Rather than
remembering their numbers, you can include the file TASKG/EQU in your
program. In addition to TCB offsets, flags and error codes, it defines
the following labels:

    XML0   EQU  >B0         XML base (table is at >A000)
    INITSK EQU  XML0+1      Initialize scheduler
    FORK   EQU  XML0+2      Create a new task
    KILL   EQU  XML0+3      Kill a task
    DIE    EQU  XML0+4      Kill current task
    YIELD  EQU  XML0+5      Return control to the scheduler
    GRAB   EQU  XML0+6      Grab a semaphore
    WAIT   EQU  XML0+7      Wait for a semaphore
    BARIER EQU  XML0+8      Wait at a barrier
    DROP   EQU  XML0+9      Drop a semaphore
    RAISE  EQU  XML0+10     Raise a semaphore
    GETTCB EQU  XML0+11     Get TCB pointer from task ID
    ENDTSK EQU  XML0+12     Terminate multitasking

The corresponding call becomes, for instance:

           XML  INITSK

For detailed instructions on the use of these routines, see the chapter
on multitasking assembly language. What follows here is just a quick
description of how to call each XML routine.

####  XML \>B0 Call any multitasking routine

This is the common entry point to all routines. It allows you to
overwrite the remaining entries in the XML table, in case you need them
for your program.

Byte \>834A needs a routine number, from 1 to 11. This number
corresponds to the second digit of the XML number, as described above
(but the first digit is ignored anyhow, so you can use the same labels).
Parameter passing is the same as for a direct call via a dedicated XML.

Example:

           ST   INTISK,@>834A     routine to be called
           DST  >FFFF,@>834C      pass parameter
           XML  >B0               call it

####  XML \>B1 call INITSK

Word \>834C contains the TCB pointer, or \>FFFF to use the default TCB.

You can have upto 32 tasks running, including the parent program.

####  XML \>B2 call FORK

Word \>834C contains the TCB pointer (in cpu memory).

Upon return, byte \>834B will contain the task ID, or \>00 if an error
occured (you can get the error code from word TSKERR)

This XML can be used to create either assembly or GPL tasks. To start an
assembly task, you must specify its address in the TPC entry of the TCB
(the task will generally start with a `LIMI 2` instruction). If you
don't, the XML wrapper assumes you want to start a GPL task and clears
the TWR and TPC entries in the TCB.

If you would like the child GPL task to start at a different GRAM
address, you must set this address in the GAD file of the TCB. If you
set this field as \>FFFF, the scheduler will substitute the current GRAM
address for it (which means the child task will return after the
`XML >B2`).

If the child task returns after the XML, it will have the Cnd bit set.
The parent task returns with the Cnd bit reset, so you can tell parent
from child with a `BR `(resp. `BS`) instruction.

####  XML \>B3 call KILL

Byte \>834B must contain a valid task ID, or \>00 for the current task.

Invalid ID errors return with the Cnd bit set and can be trapped with
`BS`. TSKERR will contain the value RNOTID.

####  XML \>B4 call DIE

No parameter is required.

####  XML \>B5 call YIELD

No parameter is required.

The instruction returns with the Cnd bit set if a task switch
effectively occured, with the Cnd bit reset otherwise.

####  XML \>B6 call GRAB

Word \>834C must contain the address (in cpu memory) of the semaphore to
be grabbed.

The Cnd bit will be set if the semaphore was grabbed immediately and
reset when the task hanged for a while before the semaphore became
available.

####  XML \>B7 call WAIT

Word \>834C must contain a pointer to the semaphore to be waited for.

Just like the above, a reset Cnd bit indicates that no waiting occured,
a set bit signals that the task hanged for a while.

####  XML \>B8 call BARIER

Word \>834C must contain a pointer to the semaphore controlling the
barrier.

Here also, the Cnd bit can be used to detect whether the task hanged at
the barrier (jumps on `BS`) or went right through (jumps on `BR`).

####  XML \>B9 call DROP

Word \>834C must contain a pointer to the semaphore to be dropped.

`BS `can be used to detect cases when this caused a task switch,
`BR `for cases when the task continued normally.

####  XML \>BA call RAISE

Word \>834C must contain a pointer to the semaphore to be raised.

This will rarely cause a task switching (unless the semaphore value was
negative), but if this is the case the Cnd bit will be set upon return.

####  XML \>BB call GETTCB

Byte \>834B must contain a valid task ID, or \>00 for the current task.

Upon return, word \>834C will contain the task pointer, or \>0000 if the
task ID was invalid. Such an error sets the Cnd bit and can be detected
with a `BS`.

If \>834B contains \>FF, the word \>834C will contain a pointer to the
task list upon return.

####  XML \>BC call ENDTSK

No parameter is required.

Revision 1. 7/28/01. OK to release
Revision 2. 9/9/01. Minor changes to Basic scheduler

[Back to the TI-99/4A Tech Pages](titechpages.md)
