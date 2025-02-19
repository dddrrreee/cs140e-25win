## Equivalance checking finale.


<p align="center">
  <img src="images/pi-threads.jpg" width="700" />
</p>

### clarifications and notes

***BUGS***:
  - the comment for `full-except-asm.S:syscall_full` is wrong:
    you *must* set the `sp`.  (Fortunately the tests seem to catch this
    dumb mistake.)

NOTE:
  - You have to drop in your own `breakpoint.c` from lab 10
    and delete ours from the `Makefile`.  (You might have put these
    `breakpoint.h` implementations in `mini-step.c` which is fine.)

  - If you do a pull, there is a `code/switchto.h` that got added
    with a better type signature for `cswitch`.

  - The threading code as checked in only checks user level threads
    so as a result *only checks user code* --- it won't be checking
    the privileged routines out of the box.   

    You're strongly urged to do so as the "interesting" code you write.
    (See below).


### overview

Today's lab is a mid-term checkpoint.
By the end of the lab we will verify that the breakpoint and
state switching assembly you wrote 
in the last lab is correct by:
 1. Dropping it into a crazy pre-emptive threads package;
 2. Use single step hashing verify 
    that the threads behave 
    identically under all possible thread interleavings.

Next lab we will use this same basic approach to also verify that
virtual memory works.

Mechanically:
 1. You'll drop the assembly routines you wrote last time one at a time 
    replacing ours and check that the lab 10 tests still work.  You will
    be able to swap things in and out.

 2. Once that works run the threads and check hashes (more on this later).


Checkoff:
  1. `make checkoff` works in `code`.  You have to delete
     our `staff-breakpoint.o` , `staff-switchto-asm.o` and
     `staff-full-except-asm.o`.  Note, you built the `breakpoint.h`
     interface in lab 10.

  2. You have a final project description turned in.

  3. You do something interesting cool with the threads package.
     See Part 2 below.

     One big option: run in privileged mode with matching (discussed
     in Part 2, also counts as an extension).

There are absolutely a bazillion extensions.  You can easily
do a final project based off this lab.
  
----------------------------------------------------------------------
### Part 1: start replacing routines and make sure tests pass.

The code for this part is setup so you can flip routines back and forth.
As is, everything should pass "out of the box"

Maybe the easiest way to start:
  1. Go through `switchto-asm.S`, in order, rewriting each routine to
     use the code you wrote in the last lab. (I would comment out the
     calls to our code so you can quickly put them back.) Make sure the
     tests pass for each one!  Once all are replaced, comment out our
     `staff-switchto-asm.o` from the Makefile.

     To help understand you should look at `switchto.h` which calls these 
     routines.

  2. Go to `full-except-asm.S` and write the state saving code.  This should
     mirror the system call saving you did last lab. 
     Make sure the tests pass for each one.  Once all calls to our code     
     are removed (I would comment them out so you can flip back if needed)
     remove the link from the Makefile.

     To help understand you should look at the `full-except.c` code which
     gets called from it.

  3. Replace our `staff-breakpoint.o` with yours from lab 10.
     You might have put that code in your `mini-step.c` which is
     fine.

How to check:
 - You should be able to run "make checkoff".  This will compare all
   the 0, 1, and 2 tests to their out files.  It will also run tests
   3 and 4 which we can't compare outfiles to but have internal
   checks.

 - You can save time by just running 3 and 4.  If these pass I'll
   be shocked if 0, 1, 2, fail.  It's that these are easier to debug.


NOTE: 
 - will be checking in more description.

----------------------------------------------------------------------
### Part 2: do something cool / useful with your pre-emptive threads

Do something interesting or cool with the equivalance threads.
Some easy things:


  - We always switch every instruction.  This won't necessarily
    find all bugs.  It's good to have a mode than can randomly switch.
    Perhaps every 2 instructions on average, then every 3 instructions
    on average, etc.   But you can make whatever decision seems
    reasonable.  

    If you recall in lab 11 we used `pi_random()`.  You'll need 
    to include:

            #include "pi-random.h"

    Also, `pi_random()` uses division, and the ARM doesn't have it.
    So we also have to include a gcc library to emulate it by adding
    the following to the end of the `code/Makefile`

            LIB_POST += $(CS140E_2025_PATH)/lib/libgcc.a

    Before the line:

            include $(CS140E_2025_PATH)/.../Makefile.robust

    Finally(!), do a git pull of libpi to update the makefile.

    And can then just call it:

            uint32_t v = pi_random();

     
  - You could add a yield (like your `rpi-threads.c` had) 
    and some tests to check that it works.  Potentially
    can add other routines (such as `wait()`).  This 
    involves adding new system calls.  AFAIK these should
    work fine with hashes.

  - Add a working lock, trylock or other mutual exclusion
    system calls.

  - A bigger thing (which counts as an extension) is to do your
    interleave checker for real now that we have threads --- we
    can currently check an arbitrary number of threads.  You can
    spin this up into a final project.

  - Another cool thing (not exactly trivial) is to run multiple
    threads using the same stack by copying their stack in and out on
    switch (a form of swapping).  It lets you run multiple routines at
    the same location with the same stack and so get the same hashes.
    As a side effect this will make it clear why we use VM, but is
    actually cool.


  - Add timer interrupts and check that the hashes remain the same.
    Note: You will need to enable interrupts in the thread's CPSR.
    This will change the hashes.  The general way to handle this is to
    remap the CPSR bits during hashing: (1) verify that the current
    CPSR has interrupts turned on (7th bit is 0), (2) make a copy of
    the regs to hash, (3) disable interrupts in the copy of the CPSR
    (7th bit set to 1), (4) hash the registers.

    (Re-mapping CPSR bits let's you play around with machine state
    but still validate that nothing changed in the equivalance
    hashes.)

    A few notes:
      - Our libpi isn't thread safe.  The easiest way to handle this
        is to leave the CPSR disabled in kernel mode.  However, depending
        on how long handling debug exceptions takes, this means you will
        always get a timer interrupt on every instruction.  You might
        need to increase the timer period significantly.  In any case
        should check how many interrupts occur during each thread to
        make sure you have them on at user level.

      - You should verify that interrupts are off for
        your prefetch handler and for your system call handler for 
        similar reasons.

      - You'll have to add a routine to `staff-full-except.c` to 
        handle interrupts.

  - Even better than all the above is something cool you come up with!

With all that said the best thing to do is the following:

----------------------------------------------------------------------
### The best Part 2

The absolute best thing you can do --- and considered a major extension
--- is to also test your privileged switching and exception trampolines.
The way you do so is to run the threads at SYSTEM level (privileged)
but use breakpoint *matching* (which works at privileged) rather than
*mismatching* (which does not).

The challenge here is that unlike mismatching, for matching
you have to know the exact pc to match on. 

The easiest quick and dirty hack is to run on on code that does
not branch --- this means that the next PC to match on is the 
current PC+4 bytes.

Fortunately we have a bunch of routines in `staff-start.S` 
that don't use branch at all.

So the basic algorithm:
  1. Fork the routines in `staff-start.S` and add a boolean to
     the thread control block indicate you are running in matched mode.
     You should probably also check the CPSR and if its in USER mode
     assert that that match mode is false and if its SYSTEM mode that
     its true.

  2. In order to check that you get the same hash, before hashing
     in the match handler make a copy of the registers,
     and swap modes from SUPER to USER and then hash.  You should
     get the same answer as your mismatch runs.
  3. Note: you will have to change some assertions that check
     that system calls only occur at user mode so that they work
     at both USER and SYSTEM mode.  You'll have to change the
     `full-except.c` code to patch the registers.
  4. You'll also have to add routines to match and disable matching.
     Note, that you should do matching on `bvr1` and `bcr1` since
     single stepping is using `bvr0` and `bcr0`.  If you don't
     do this then as soon as the code calls mis-match, your matching
     will get overwritten and stop working.

  5. Common mistake is to miss a place to put the matching calls:
     you should search through the code for mismatch calls and make sure
     these are paired with match calls for threads in match mode.

The less hacky way which is even more of a major extension is to
handle code that branches by:
 1. Run a routine at USER level in single stepping mode, 
    recording the instructions it ran in a per thread array.
 2. Rerun at USER and make sure you get the same trace.
 3. Then rerun the routine at SYSTEM mode, but instead of mismatching
    do matching (using the addresses in the array).

#### A different method: rerun one instruction.

A less code (I think) but more IQ intensive method to test privileged
and unpriviledged switching is to rerun each instruction at a privileged
mode after you get a mismatch exception and check that it produces the
same registers.

This would work as follows:
  1. Add a `reg_t` field to the thread block that holds a copy
     of the previous registers (e.g., `reg_t prev_regs;`). 
  2. Run each thread at USER as normal;
  3. Each time you get a mismatch exception:
     1. Record the registers passed to the exception handler.
     2. Rerun exactly that one single instruction using
       `prev_regss` at a higher privilege;
     3. Compare the registers from (ii) to those from (iii)
        and make sure they are the same.
     
        (Note this will require some cleverness since this
        will be two different exception invocations.)

Note:
  - The way our exceptions work, we don't handle recursive 
    faults.  You could change this. Or you could record
    enough state that you can remember where you were.
  - This technique won't always work with device memory.
  - Also potentially won't work if the instruction modifies
    memory in a way that running it a second time gives a different
    answer.  The easiest way to start is with the stackless
    routines in `staff-start.S` which don't write memory.  You
    can then scale up to more complex.  

The cool thing is that once you have this method working, you can
then repurpose it to detect which instructions are not virtualizabe by
turning it into an automatic checker that detects when an instruction
fundamentally behaves differently at user mode and privileged mode.

The ARM has several of these instructions buried in its massive
ISA, so it's great to have an automatic method for finding them.

There's not enough information here, so ask if this seems potentially
interesting :)

----------------------------------------------------------------------
### Summary

Mistakes in context switching and exception trampolines are some of
the hardest you can hit.  This lab has shown how to find them fairly
easily by exploiting low-level debugging hardware.  You can generalize
this approach to check many OS and non-OS properties.  (We will use it
throughout virtual memory.)

There are tons of final projects you can do based off of this lab.
  - Combine it with your interleave checker.
  - Tune the code aggressively  relying on the fact that
    equivalance hashing detects bit-level mistakes.  Am curious how much
    faster you can make it!
  - Many others.
