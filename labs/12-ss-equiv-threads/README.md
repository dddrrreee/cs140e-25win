## Equivalance checking finale.


<p align="center">
  <img src="images/pi-threads.jpg" width="700" />
</p>

### clarifications and notes


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

Will add a bunch of suggestions but you can do your own.
