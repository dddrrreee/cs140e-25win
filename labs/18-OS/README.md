## equiv OS.

Today we'll have a very small, OS that does equivalance checking.
It provides `fork`, `waitpid`, `exit` and `sbrk`.

Some news:
  - The bad news: README is more of a WRITEME.
  - The good news:  lab today is just an extension.

Currently it loads a simple hello world program and runs it,
checking that you get the same result with and without VM.

BUGS:
  - if you get duplicate symbols: if it's in a assembly file,
    you can add a `.weak` directive for that name so the 
    linker doesn't complain.  E.g.,  to make `foo` weak:

    ```
	        .weak	foo
        MK_FN(foo)
            ...
    ```

    if it's in C code, you can do 
    ```
        // in eqx-os.c
        __attribute__((weak)) cp15_ctrl_reg1_t
        cp15_ctrl_reg1_rd(void) {
            return staff_cp15_ctrl_reg1_rd();
        }

    ```


High Level:
  - We take the interleaving single-step code from lab 12 and
    add pinned VM.
  - We add full, distinct user level programs (see: `user-code`).  
    How they are different from non-user programs:
       - These do not use libpi at all.  Instead all interactions 
         with the pi are through system calls.  We do this so 
         we can prevent the user code from directly accessing
         the hardware and messing up other programs.

       - User programs are linked with a custom linker script 
         (`user-progs/small-proc.ld`) that does two things.

         First, the script splits the user program into two 1MB segments:
         one for code (at `0x400000`) and one for data (at `0x50000000`).

         Why: (1) we want the user program's virtual addresses to not
         conflict with with the kernel (2) we want code and data seperate
         so that it's easier to make fork faster (b/c the read-only code
         page can be shared versus copied).

         Second, the original linker script we used assumed the program
         itself would set up its environment.  The worked great for us
         since our code was the only thing running.  However, for
         user programs, the OS needs to be able to look inside and
         see where the code, bss, and data is as well as their sizes so
         that it can lay them out in virtual memory.
        
  - We add support for loading user programs.  There's several ways to
    do this (including pulling them off the microSD using your fat32).
    We pick the absolute simplest to understand: we have a simple program
    (`code-gen.c` in `user-progs/code-gen-src`) that takes in a the user
    program's ".bin" file and emits it as a byte array. This array can
    be linked into test programs.  Its downside is that when you change
    the user code, you have to relink the OS, but the upside is that it's
    pretty simple to understand.  

    The great thing about single step equiv checking is that once
    you have the hashes, you can start rewriting the OS and just check
    that no hash changes. 

What to do right now:
  - change the makefile to just run test 4 (this is the same test
    from lab 12).   make sure it runs as before (smearing hello
    8 times since there are 8 threads).

  - change the makefile to just run test 5 and make sure it 
    does a similar smearing.

  - Go through the code, starting in `code/eqx-os.c` and the user
    programs in `user-code`.  The `tests` directory has the old
    tests from lab 12 as well as one new test `tests/5-user-prog.c`
    showing how to do user program loading.  

    It uses the byte array produced from user-code/0-hello.c and passes it
    to the internal exec call to load it.  Runs it once to get the hash.
    Creates 5 and checks the hash is the same.  I'd follow this around,
    maybe make some small changes, and make sure the hashes don't change.
  
  - Add segment free's on exit if the process has allocated pins.
  - Make sure this gives you the same result when you run over and
    over.
  - Add some other system calls, e.g., fork, waitpid.  Write some
    other test programs.

  - You should be able to drop in all your code from the class,
    replacing ours and see that the tests pass.

Once you have hashes, one general game is to find stuff that should not
lead to user-visible differences and do it.  Some options:
 - Drop in page tables. 

 - On each single step exception: optionally turn the MMU on
   and off.  This should have zero effect on the the result,
   but can spot cases where you don't flush things correctly
   (esp if you have the data cache on).
 - Currently the code uses a single ASID for all processes.
   You can change this by adding multiple asids (nothing should
   not change).  

   When you have page tables you can do many checks with ASIDs: first use
   one and always flush when switching, then use one for each process,
   run enough so that you reuse some to check for mistakes.

 - One easy thing you can do: artificially restrict the number 
   of 1mb pages available, so that physical pages get recycled
   more often: this can expose places where your cache flushing
   code doesn't work.

 - Add timer interrupts: should not effect the other hashes.


 - Make the permissions more accurate.  Add different domains
   for different processes.

 - Turn on the icache and dcache.  One issue: after copying code
   you'll have to flush both (either entirely or by range) since
   they are not coherent.
  
 - Major great thing: rewrite the code (or some of the code) so it doesn't
   suck.  This is what I will be doing. The great thing about equiv is
   that it's super brutal at finding weird errors.

A fun fetchquest:  Add more system calls!
 
 - Add fork, waitpid, pipes, your fat32, signal handlers for address
   exceptions, sbrk for malloc, etc

   This is always fun b/c it's a straight up fetchquest.  It also
   very useful b/c the more programs you have, the more hashes you'll
   have to check.  And, obviously, you can run each to get its hash in
   isolation, then with everything to make sure the hashes don't change
   in composition.

   Ideally: you should be able to have the identical code run on your
   laptop and the pi.

More fun: Make stuff fast!  In general, any speedup you do to the kernel
should not change any hash.  
 - Instead of flushing entire caches to it by address range.  should
   have no change.
 - Instead of flushing entire tlb, just do by asi, should have
   no change.
 - Use different compiler options.
 - In general: measure how many microseconds an entire test takes,
   and try to speed up.  Shouldn't be hard to hit massive 10x wins
   since we are aggressively stupid.

Make ultra-fork:

 - Try to have 100s of thousands of processes.  The key here:
   Make the program use less memory.  Rather than two big 1MB pages,
   initially use 64k or even 4k pages and only grow (promote)
   if needed.    

 - Use the small memory to make fork really fast.  I *believe*
   you can beat your laptop OS in number and speed of processes
   hopefully by 10x.

