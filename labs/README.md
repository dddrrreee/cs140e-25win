## The labs


<p align="center">
  <img src="lab-memes/coding.jpg" width="400" />
</p>

Below describes where we're going and why.   The labs themselves have a
lot more prose on each topic.  There is a pointer to where we currently
are: the labs after this point can still see significant revisions.

  - Labs usually have some [crash-course notes](../notes/README.md) about
    key concepts --- you must read these before the lab or you'll be lost.
    We also suggest re-reading some number of labs later after the concepts
    have sat for a bit so you get a deeper view.  Note summary (so far):

  - For those with high executive function, there is an initial list
    of possible [final projects](./final-projects/README.md).

  - For those with low executive function there is an initial set of 
    [lab-memes](./lab-memes) revolving around code, bugs, jobs, pizza.

A note on lab structure:

  - The labs are deliberately *not* organized as a
    self-contained set of blocks --- we don't do all the labs for topic A,
    then all the labs for topic B, etc.  Instead, we break up each topic
    into pieces and do some of A, then some of B, then circle back to A,
    then circle back to B, etc.

  - This approach is based on newer methods from learning
    theory ("spaced repetition") that (hopefully) make the topics sink
    in deeper.  They are also used to break up hard topics with easier
    (but not easy) "breather" labs to give more time to get stuff under
    control and consolidate.

---------------------------------------------------------------------
### 0: non-pi hacking

Unlike all subsequent labs, our first two don't use hardware.  They should
give a good feel for whether the class works for you without requiring
a hardware investment.

  - [0-intro](0-intro): the intro (non-lab) lecture.

  - [1-trusting-trust](1-trusting-trust): Ken Thompson is arguably our patron
    saint of operating systems --- brilliant, with a gift for simple code that
    did powerful things.   We will reimplement a simplified version of a crazy
    hack he described in his Turing award lecture that let him log into any
    Unix system in a way hidden even from careful code inspection.

---------------------------------------------------------------------
### 1: Going down to metal (part I)

The first few labs will writing the low-level code needed to run the
r/pi and using modern techniques to validate it.  Doing so will remove
magic from what is going on since all of the interesting code on both
the pi and Unix side will be written by you:


  - [2-gpio](2-gpio/):  Two parts.  First, we will give out the
    hardware and make sure it works: [0-pi-setup](0-pi-setup/README.md).

    Second start getting used to understanding hardware datasheets by
    writing your own code to control the r/pi `GPIO` pins using the
    Broadcom document GPIO description.  You will use this to implement
    your own blink and a simple network between your r/pi's.

    ***READING***:
       - Note: [GPIO](../notes/devices/GPIO.md).
       - Note: [crash course in writing device code](../notes/devices/DEVICES.md).
       - pages 4--7 and 91---96 of the broadcom
         datasheet (`docs/BCM2835-ARM-Peripherals.annot.PDF`)

***We are here*** ===>

  - [3-cross-check](3-cross-checking): you will use read-write logging
    of all loads and stores to device memory to verify that your GPIO
    code is equivalent to everyone else's.  If one person got the code
    right, everyone will have it right.

    A key part of this class is having you write all the low-level,
    fundamental code your OS will need.  The good thing about this
    approach is that there is no magic.  A bad thing is that a single
    mistake can make a miserable quarter.  Thus, we show you modern
    (or new) tricks for checking code correctness.

---------------------------------------------------------------------
### 2. Execution: threads, interrupts, exceptions (part I)

Execution comes in many forms.  It can be a tricky topic both because
of its thickets of low-level, hardware specific details and because of
how hard mistakes are to debug.  You will build and use four different
versions of execution (threads, interrupts, exceptions and processes)
so that you understand in a depth-perceptive way what is essential and
what is incidental.  We will also do multiple new tricks using these
that work well at finding bugs in novel ways.

After four runs at the architecture manual, you will have a new
comfort with using it.


  - [5-interupts](5-interrupts/): 
    you will walk through a simple, self-contained implementation of
    pi interrupts (for timer-interrupts), kicking each line until you
    understand what, how, why.  You will use these to then implement
    a simple system call and a version of `gprof` (Unix statistical
    profiler) in about 30 lines.  Finally, you'll write a user-level 
    system call implementation (both the code two switch from privileged
    to user mode and the code to handle system calls).

    Perhaps the thing I love most about this course is that because we
    write all the code ourselves, we aren't constantly fighting some
    large, lumbering OS that can't get out of its own way.  As a result,
    simple ideas only require simple code.  This lab is a great example:
    a simple idea, 30 lines of code, an interesting result.  If we did
    on Unix could spend weeks or more fighting various corner cases and
    have a result that is much much much slower and, worse, in terms
    of insight.

    Incomplete reading list: 
      - [armv6 interrupt cheat sheet](../notes/interrupts/INTERRUPT-CHEAT-SHEET.md) 
      - [caller/callee](../../notes/caller-callee/README.md)
      - [example mode mistakes](../notes/mode-bugs/README.md)
      - [using gcc to figure out assembly](../notes/using-gcc-for-asm/README.md)
      - `5-interrupts/docs/BCM2835-ARM-timer-int.annot.pdf` --- excerpt from the Broadcom
        document, discusses how to enable both general and timer interrupts.

      - `5-interrupts/docs/armv6-interrupts.annot.pdf` ---  excerpt from the ARMv6
        document in our main `doc` directory.  Discusses where and how
        interrupts are delivered.

  - [6-threads](6-threads):  we build a simple, but functional
    threads package.  You will write the code for non-preemptive context
    switching:  Most people don't understand such things so, once again,
    you'll leave lab knowing something many do not.   This will give you
    a second view of execution (and some depth perception of the topic)
    as well as a more fluent handle on assembly code.


---------------------------------------------------------------------

<p align="center">
  <img src="lab-memes/bug.jpg" width="400" />
</p>

