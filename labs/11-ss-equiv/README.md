## Pre-emptive threads, checked with single step equivalance 

<p align="center">
  <img src="images/pi-ss-equiv.jpg" width="700" />
</p>

NOTE:
  - If you see a mistake: refetch the `README.md`.  If that 
    doesn't fix it let us know!

By the end of this lab you'll have your own simple pre-emptive threads
package and it won't actually be that hard.  Crucially, it will *work*
to the point that we'll be surprised if your code has a bug in it.
The reason for this strong claim is that you will use the debugging
hardware to verify that running code pre-emptively gives exactly the same
results as running it sequentially --- that every register has exactly
the same value for each instruction that gets executed.  Given the pieces
you've built so far, building this "single step equivalance"  checking
won't be hard and yet will make hard things easy because it makes them
trivial to completely check against a simple reference.

Next week you'll be able to combine this threading system with the 
virtual memory you build to make user level processes that can implement
the UNIX system calls `fork()`, `exec()`, `exit()`, `waitpid()` etc.
You'll also use your single-step equivalance code to validate that this
combination works by using it compute the equivalance hash of processes
running without virtual memory and pre-emption and verifying you get
the same hash result when running with virtual memory, pre-emption.

  - NOTE: this trend of comparing the equivalance hash of code run
    in the simplest way possible to code running with full complexity
    will be a common one for the rest of the quarter and is the only way
    we know to have 80 people build their own OS and make it surprising
    if the code is broken.

    With that said, equivalance checking isn't verification, but it does
    validate so thoroughly that it's interesting to the staff if your
    code is broken but can pass equivalance checking.  (You should let
    us know if this ever occurs!)

The specific code you'll write:
  - Your own low-level pre-emptive context switching and exception
    trampoline code that will replace ours from the last couple of 
    labs (`full-except-asm.o` and `staff-switchto-asm.o`).
  - The equivalant hashing code.
Both of these will be fairly short.  The payoff at the end is a simple
pre-emptive thread package that works.

Checkoff:
  1. `make check` passes in `1-code` with all tests.
  1. `make check` passes in `2-code` with all tests.

---------------------------------------------------------------
## Background: pre-emption and context switching.

Today you'll implement the code needed to support pre-emption.
Pre-emption refers to forcibly interrupting a computation (for example
using a timer-interrupt or debug exception).

The threads package you wrote in lab 5 was non-preemptive ("cooperative")
in that threads ran until they yielded the processor (e.g., by calling
`rpi_yield()` or `rpi_exit()`).  Cooperative threading has two big
positives:
  1. It is relatively simple to build: just save and restore the 
     callee-saved registers.
  2. It is relatively simple to use: by default, the code being run by a
     given thread is a non-interruptible, "critical section" that
     can't be interrupted by another thread that pre-empts it.  Thus,
     you can view cooperatively threaded code as being made of large
     atomic sections only broken up by voluntary yields.

Unfortunately, we can't generally rely on user processes to run 
cooperatively:
  1. Even one infinite loop would lock up the system.  We need some way
     to bound how long the user process can run before the kernel gets
     back control.
  2. Devices won't generally work well.   Many devices have small
     queues (e.g., UART) or tight timing requirements (for correctness
     or speed).  If the kernel can only check a device when the user
     yields then at best throughput and latency will suck and at worse
     it would lose data and gain bugs.

Thus, we want to two things:
  1. To be able to run user-processes pre-emptively where they can be
     forcibly interrupted (e.g., by a timer-interrupt, device interrupt
     or debug hardware exceptions).

  2. When we interrupt a process we want to *completely* save its state
     --- all 16 general purpose registers and CPSR --- so we can switch
     to another process by loading its state --- also, all 16 general
     purpose registers and CPSR.

You've already done many labs where you've interrupted running code
(step 1).  The only new thing for this lab is saving and restoring
*all* registers (callee and caller, as well as the CPSR) rather than
just saving:

  - Only the caller-saved registers, as we did for interrupts
    where we returned back to exactly the code that got interrupted
    rather than switching to another thread or process after the interrupt
    was done.

    (Recall: we saved caller registers before calling the C interrupt
    handler because the compiler would assume it could trash any caller
    register and we don't know which ones were in use.  We didn't have
    to save the callee-saved because the C compiler would do so before
    using them.)

  - Only the callee-saved registers, as we did for cooperative threads
    where because the thread code called the yielding routine itself the
    C compiler would know the caller-saved registers were potentially
    trashed so would save and restore them itself. (Useful question:
    Why did we need to save callee since the C code would?)

You've already saved and restored registers before in isolation so
conceptually its not hard to save both.  However, the ARM has the
complication that:

  1. It has different "modes" for user level, kernel level and
     different exceptions.  Each of mode has private copies of some
     registers (for today: sp and lr).  These are called "banked"
     registers or shadow registers.   

  2. When an exception occurs we will be at a different
     mode than the code that got interrupted.

Thus, you'll have to write code to read and write the registers at one
level from another --- for the most part, the unprivileged user stack
pointer (sp) and return register (lr) from a privileed interrupt context.

#### Banked registers

<p align="center">
  <img src="images/banked-registers.png" />
</p>

#### cpsr layout

<p align="center">
  <img src="images/cpsr-layout.png" />
</p>

---------------------------------------------------------------
## Part 0: Reading and writing user registers 


This lab depends on correctly figuring out low-level machine facts: how
to change modes, how to access registers in one mode from another, etc.
So we'll first do these parts in isolated, easy to debug pieces.

As we mentioned above, in order to save and restore the state of a level
processs from privileged mode we'll have read and write the two banked
user-mode registers (`sp` and `lr`).  The set of tests have you implement
the code to read and write these registers in three different ways.

The code you write:
  - `0-user-sp-lr-asm.S`: contains all the code to write. 


There are four test harnesses.  You don't have to modify them though you
should definitely insert assertions or print statements if things act
weirdly!  Look at the comments in the files and the tests and implement
each part at a time.  This is a pretty mechanical fetch quest.

You should do them in the following order:

  1. `0-cps-user-sp-lr.c`: get the USER SP and LR registers
     by using the `cps` instruction to switch to `SYS_MODE` (which has the
     same registers as USER) and back.  The `0-user-sp-lr-asm.S` file has
     a example for `cps_user_sp_get` but you should fill in the others.

```
        @ 0-user-sp-lr-asm.S: get USER mode's sp and return the 
        @ result in r0.
        MK_FN(cps_user_sp_get)
            cps #SYS_MODE
            prefetch_flush(r1);   @ note we have to use a caller reg
        
            @ USER And SYSTEM share the same sp/lr/cpsr
            mov r0, sp
        
            cps #SUPER_MODE
            prefetch_flush(r1);
        
            bx lr

```

  2. `0-mem-user-sp-lr.c`: do the same as (1) but use the `ldm` and
      `stm` instructions with the carat operator "^" to access the
      user registers.  The routine `mem_user_sp_get` has an example:

```
        @ store the user mode sp into the address passed as the 
        @ first parameter (i.e., in r0)
        MK_FN(mem_user_sp_get)
            @ dereference the address held in <r0> 
            @ and write the 32-bit result into user 
            @ mode <sp> register.
            stm r0, {sp}^
            bx lr
```

  3. `0-any-mode-sp-lr.c` generalize the mode switching method for
     any mode (other than USER) so that you can read and write registers
     at any privileged mode.

     You should use the `msr` instruction with the `_c` modifier to to set
     the `cpsr` mode using a register:

            msr cpsr_c, r0


     Followed by a prefetch_flush that uses a caller saved register you
     don't care about.  You can see an example of where we use `msr` in
     `libpi/staff-start.S`

---------------------------------------------------------------
## Part 1: using `rfe` to switch to user level

Here you'll implement different examples that use the `rfe` instruction.
The `rfe` instruction lets you simultaneously set both the `pc` and `cpsr`
at once and is a key part of how you context switch into a user process.

Unlike your thread context-switch code you can't switch into a user-level
process by simply setting the `pc` register because you *also* have to
change the `cpsr` to switch from privileged kernel mode to unprivileged
`USER` mode.  If you did this mode switch before jumping to user code
you'd get a privileged fault (since the `pc` would still be in kernel
memory).  And doing it after has similar issues. The `rfe` instruction
lets you set both at once.

To see how `rfe` works:
  1. RTFM the manual.
  2. Then look at the complete example `1-rfe-example.c`, with associated
     assembly code in: `1-rfe-asm.S:rfe_asm` and
     `1-rfe-asm.S:rfe_trampoline_asm`.  Make sure the comments and output
     make sense or the next 6 lines of code could easily take a few hours.

There are two tests.  Since messing up assembly can be hard to debug,
we split restoring registers into one small change (test 1), and then
a larger one (test 2):

 1. `1-rfe-blk.c`.  This changes the example we give you
    `1-rfe-example.c` in a minor way to take a 17-entry array of
    32-bit words (as you would use with process switching) instead of
    a 2-entry one.

    What to do: You should write the code `1-rfe-asm.S:blk_rfe_asm`
    to handle a 17-entry array with the `pc` at word offset 15 (byte
    offset 15*4), and the `cpsr` you want to restore at word offset 16
    (byte offset 16*4).  This differs from our example where the pc
    was at offset 0 and cpsr was at word offset 1 (so byte offset 4).
    All you have to do is add the right constant value to the sp register
    before doing the rfe instruction.

    (NOTE: It's a trivial change, but you want this correct before doing
    the next step.)

 2. `1-rfe-switchto-user.c`: This step sets all the registers in the
    17-entry array and eliminates the need for a trampoline used in
    `1-rfe-asm.S` to setup the stack pointer.

    What to do: You'll write `1-rfe-asm.S:switch_to_user_asm` to load
    user mode registers `r0-r14` and then do an `rfe` to load r15 and
    the CPSR.  This is just a matter of copying and modifying some of
    your part 1 assembly.

    ***Note, the test in its current form only validates `r0-r3`, `sp`,
    `pc` and mode (we will do the others below).***

At this point you have a partially validated register switch: we'll
do the rest of it in the next few stages.

------------------------------------------------------------------
### Part 2: saving all user registers: `2-reg-save-asm.S`

***NOTE: If you see this, do a git pull: we probably cleaned up
the prose ***

Here you'll implement the assembly to save all registers in ascending
order into a single 17-entry block of memory on the exception stack and
pass that to the system call exception handler.  This is the other half
of getting to a full processes.

There are two different tests:
  1. `2-reg-save.c`: The simplest test harness, which
     uses the `rfe_asm` (from part 2) to run user code that calls a
     system call handler.

     What you write: `2-reg-save-asm.S:`swi_trampoline`.  This is the
     assembly code that will save all registers into a 17 entry block on
     the interrupt stack and pass the base of this array into the system
     call routine (`2-reg-save.c:do_syscall`) which will simply print them
     and reboot.  Make sure you increment and compute offsets correctly.

     The idea here is that we can check that your register save
     worked by writing assembly code to set each register to 
     a known value and that this is what is saved. 

     Specifically, the assembly code in the routine
     `1-code/staff-start.S:mov_ident` set each register with its number
     (it sets r1 to 1, r2 to 2, ..., r14 to 14) and then invokes the `swi`
     instruction, triggering a system call exception.  This makes saving
     easy to debug: `r0-r14` in the 17-entry register block should have
     its offset as a value.  `r15` should be the pc of the swi instruction
     (for me `80e0`) and r16 should have `USER` as a mode.

     If you look at the system call handler `2-reg-save.c:do_syscall`
     you can see that it dumps the registers, so that you can check them.

  2. `2-reg-save-full.c`: The more fancy testing harness, which works
     similar to above, but instead of manually setting the general purpose
     registers using (similar to `mov_ident`) instead
     initializes a 17-entry register block and then uses your
     `switchto_user_asm` (from part 2) to load the registers and jump
     to the swi instruction directly.  Output should be the same as the
     first test.  If any register is off, you should be able to see.

At this point you have a partially tested set of user save and restore
routines.  We will now verify them using single stepping without much
work and to a degree you'd be surprised if they are broken.  

Great, now you have the main building blocks: we'll put all the pieces
above together on Thusrday, hopefully in a way that blows your mind.

---------------------------------------------------------------
## Part 3: saving and restoring privileged registers.

***NOTE: If you see this, do a git pull: we probably cleaned up
the prose ***

In this part you'll write code to save and restore registers when
you're coming from and going to privileged (not user mode).

  1. The challenge here is that the caret operator `^` *only* loads
     or stores to user mode registers, not to any other (privileged)
     mode, so won't help us here.  So, since you don't have caret,
     what do you do?   The good thing is that `0-user-sp-lr-asm.S`
     has most of the pieces already so you can just use those.

  2. In some sense it is easier to save and restore state between
     privileged modes versus user mode since we can switch back and
     forth however we want, whereas once we go to user mode we need a
     system call to get back.

  3. Probably the easiest approach to restore the privileged registers
     is to (1) do a full `msr` switch to the right mode (as in
     `libpi/staff-start.S`) and then (2) do a `ldm` of all the registers.

  4. Your privileged mode save and restore
     should work for any privileged mode, not just a specific hardcoded
     one.  So you should use `msr` not `cps`.

What to do:
  - Make copies of the two tests from part 2 (`2-reg-save.c`,
    `2-reg-save-full.c`) and the support assembly code
    (`2-reg-save-asm.S`) above and modify them to go between some non-user
    mode (presumably not `SUPER` or `SYS`).  You already have the pieces
    for this so it's mainly a re-enforcement.


Important notes:

  - If you use `msr` you'll notice the 8th bit in `cpsr` is set, but if
    you use `rfe` it is not.     This appears to be a quirk in the
    GNU assembler.  If you want to force `msr` to set all the bits it
    appears you need to have the following suffixes:

            msr   cpsr_cxsf, r1

    Or the GNU assembler will emit machine code that does not set it.
    For today we'll take the 8th bit set or not set.


***NOTE: you are allowed to do the following easier, equivalant way 
that requires less thinking***

  1. Keep the `swi_trampoline` as-is from part 2.  It assumes
     you assumes you came from user mode.  Thus, for the part 3 test
     it will save everything correctly *except* `sp` and `lr` (since it
     assumed we came from the wrong place).  So we will just fix these
     up in C code (next step).

  2. In the C code for `do_syscall`: check which mode you came from using
     the passed in register array `regs` (`cpsr` should be stored
     at `regs[16]`).

     If you came from USER keep running as before since the registers
     are saved correctly.

     If you didn't come from USER, then you want to patch the `sp` and
     `lr` locations in the reg array (`&regs[13]` and `&regs[14]`).

     This is easy because you already wrote a routine that will
     get these two registers from an arbitrary privileged mode:

            // 0-user-sp-lr-asm.S
            void mode_get_lr_sp_asm(uint32_t mode, uint32_t *sp, uint32_t *lr)

     Just call it to fix things up using the `cpsr` value saved in
     the `reg` array (`reg[16]`).

------------------------------------------------------------------
### Part 4: putting it all together: `2-code`

***NOTE: If you see this, do a git pull: we've added more prose ***
***NOTE: If you see this, do a git pull: we've added more prose ***
***NOTE: If you see this, do a git pull: we've added more prose ***
***NOTE: If you see this, do a git pull: we've added more prose ***
***NOTE: If you see this, do a git pull: we've added more prose ***
***NOTE: If you see this, do a git pull: we've added more prose ***


The code for this part is setup so you can flip routines back and forth.
As is, everything should pass "out of the box".

By the end of this part you'll have verified that the state switching
assembly you wrote in the last lab is correct by:

 1. Dropping it into a pre-emptive threads package;
 2. Use your breakpoint code from lab 10 to verify that the 
    threads compute the same register values when run sequentially
    and pre-emptively.

Next week we will use this same basic approach to also verify that
virtual memory works.

Mechanically:
 1. You'll drop the assembly routines you wrote in the previous
    parts one at a time replacing ours and check that the tests
    still work.  You will be able to swap things in and out.

Maybe the easiest way to start:

 2. Start by going through `switchto-asm.S`, in order, rewriting each
    routine to use the code you wrote in the last lab. (I would comment
    out the calls to our code so you can quickly put them back.) Make
    sure the tests pass for each one!  Once all are replaced, comment
    out our `staff-switchto-asm.o` from the Makefile.

    To help understand you should look at `switchto.h` which calls these
    routines.

  2. Go to `full-except-asm.S` and write the state saving code.
     This should mirror the system call saving you did in part 3 (above).
     Make sure the tests pass for each one.  Once all calls to our code
     are removed (I would comment them out so you can flip back if needed)
     remove the link from the Makefile.

     To help understand you should look at the `full-except.c` code which
     gets called from it.

When things pass, a good extension is to go through the code and
rewrite it, since it's currently ugly.  You should definitely go 
through and make sure you understand it.

------------------------------------------------------------------
### Summary


<p align="center">
  <img src="images/pi-goofy.png" width="700" />
</p>
