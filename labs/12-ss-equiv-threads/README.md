## Equivalance checking finale.


<p align="center">
  <img src="images/pi-threads.jpg" width="700" />
</p>

### clarifications and notes

***BUGS***:
  - the commment for `full-except-asm.S` for `syscall_full` is wrong:
    you must set the `sp`.  (Fortunately the tests seem to catch this
    dumb mistake.)

NOTE:
  - If you do a pull, there is a `code/switchto.h` that got added
    with a better type signature for `cswitch`.

  - The threading code as checked in only checks user level threads
    so as a result *only checks user code* --- it won't be checking
    the privileged routines out of the box.   

    You're strongly urged to do so as the "interesting" code you write.
    (See below).


### intro

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
  1. `make checkoff` works in `code`.
  2. You have a final project description turned in.
  3. You do something interesting cool with the threads package.
     Will add ideas.
  4. You can rerun with matching (note: this is being added,
     check back).
  5. There are absolutely a bazillion extensions (will add)
  
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
  1. We always switch every instruction.  This won't necessarily
     find all bugs.  It's good to have a mode than can randomly switch.
     Perhaps every 2 instructions on average, then every 3 instructions
     on average, etc.   But you can make whatever decision seems
     reasonable.
     
  2. A bigger part (which counts as an extension) is to do your
     interleave checker for real now that we have threads --- we
     can currently check an arbitrary number of threads.

#### Best extension

The absolute best thing you can do --- and considered a major extension
--- is to also test your privileged switching and exception trampolines.
The way you do so is to run the threads at SYSTEM level (privileged)
but use breakpoint *matching* (which works at privileged) rather than
*mismatching* (which does not).

The challenge here is that unlike mismatching, for matching
you have to know the exact pc to match on. 

The easiest quick and dirty hack is to run on on code that does
not branch --- this means that the next PC to match on is the 
current PC plus 4 bytes.

Fortunately we have a bunch of routines in `staff-start.S` 
that don't use branch at all.

So the basic algorithm:
  1. Fork the routines in `staff-start.S` and add a boolean to 
     indicate you are running in matched mode.
  2. In order to check that you get the same hash, before hashing
     in the match handler make a copy of the registers,
     and swap modes from SUPER to USER and then hash.  You should
     get the same answer as your mismatch runs.
  3. Note: you will have to change some assertions that check
     that system calls only occur at user mode so that they work
     at both USER and SYSTEM mode.  You'll have to change the
     `full-except.c` code to patch the registers.
  4. You'll also have to add routines to match and disable matching.

The less hacky way which is even more of a major extension is to
handle code that branches by:
 1. Run a routine at USER level in single stepping mode, 
    recording the instructions it ran in a per thread array.
 2. Rerun at USER and make sure you get the same trace.
 3. Then rerun the routine at SYSTEM mode, but instead of mismatching
    do matching (using the addresses in the array).

#### other extensions

Missing a bunch:
  1. do random switching.
  2. copy the stack in and out on cswitch so you can run multiple
     routines at the same location (and get the same hashes).
