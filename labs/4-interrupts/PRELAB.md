## Prelab for interrupts

For this lab, we're going to do interrupts and using them to build system
calls and device interrupts.  For this lab and for many of the other 
labs we do this quarter you'll need to have a working
knowledge of:
  1. How the ARM modes work: which registers get shared across modes
     (such as `r0`) and which registers have isolated private copies.  
     How to switch beteween privileged modes, how to switch from privileged
     to unprivileged (and back).
  2. What registers exist and their function/attributes --- e.g.,
     the stack pointer, program counter, return address.  Caller saved
     registers vs callee.

You'll use this knowledge throughout the quarter, so it make sense to
figure it out now completely to save time and pain.

The readings below go over all these aspects and more.  You can also
figure some of it out by writing C code, compiling it with gcc and looking
at the disassembled result (as we have done already for many things).

---------------------------------------------------------------------------
### Code

Super-necessary: go through all the code in:

  - [0-timer-int](0-timer-int/): this is a complete but
    concise working timer interrupt example.  You should run it and then
    go through every line to see what it does an why.  Also (important):
    modify different lines and see what happens and figure out why.
    Also (important): break it in different ways and see the symptoms
    so that you start buildig up pattern matching.

    I would cycle through it and the documents (below) to understand what
    (code), how (code+documents), and why (documents).  You'll need to go
    through it seveal times at least.  If you understand it the lab should
    be relatively straightforward.  If not, you have our sympathies.


Useful as well:

  - [0-measure](0-measure/): some simple examples of timing
    and modes.  Useful for lab.  Quick and concrete.

---------------------------------------------------------------------------
### Readings

Non-optional primary sources in the current lab's `docs` directory
(`4-interrupts/docs`):

  - `BCM2835-ARM-timer-int.annot.pdf` --- excerpt from the Broadcom document,
     discusses how to enable both general and timer interrupts.
     It's actually not too bad.

   - `armv6-interrupts.annot.pdf` ---  excerpt from the ARMv6 document in 
     our main `doc` directory.  Discusses where and how interrupts are delivered.
     You should figure out where the handler lives, and registers are written
     what values, and how to restore them.

Non-optional reading on the arm in general:

  - [INTERRUPTS](../../notes/interrupts/INTERRUPT-CHEAT-SHEET.md): this is a cheat sheet of useful page
    numbers and some notes on how the ARMv6 does exceptions.

  - [caller-callee registers](../../notes/caller-callee/README.md):
    this shows a cute trick on how to derive which registers `gcc` treats
    as caller (must be saved by the caller if it wants to use them after
    a procedure call) and callee (must be saved by a procedure before
    it can use them and restored when it returns).

  - [mode bugs](../../notes/mode-bugs/README.md): these are examples
    of different mistakes to make with modes and banked registers.
    Should make these concepts much clearer since you can just run
    the code.

-----------------------------------------------------------------------------
### Supplemental documents

Supplemental readings in the class `cs140e-24win/docs` diretory:

  1. `hohl-book-interrupts.annot.pdf`: if you were confused
     about the interrupts, this is a good book chapter to read.
  2. `subroutines.hohl-arm-asm.pdf`: this is a good review
     of ARM assembly as used by procedure calls: stack allocation,
     caller and callee saved registers, parameter passing, etc.
  3. `IHI0042F_aapcs.pdf`: this gives you a detailed view
     of the procedure call standard for the ARM.

Additional readings in the `docs` directory in this lab:

  - If you get confused, the overview at `valvers` was useful: (http://www.valvers.com/open-software/raspberry-pi/step04-bare-metal-programming-in-c-pt4)

  - There is also the RealView developer tool document, which has
  some useful worked out examples (including how to switch contexts
  and grab the banked registers): `./docs/DUI0203.pdf`.

  - There are two useful lectures on the ARM instruction set.
  Kind of dry, but easier to get through than the arm documents:
  `./docs/Arm_EE382N_4.pdf` and `./docs/Lecture8.pdf`.

If you find other documents that are more helpful, let us know!

Background information on ARM inline assembly (we will use this throughout
the quarter):

  - [arm assembly quick ref](../../docs/arm-asm-quick-ref.pdf)
  - [gcc inline assembly introduction](http://199.104.150.52/computers/gcc_inline.html)
  - [gcc arm inline assembly cookbook](../../docs/ARM-GCC-Inline-Assembler-Cookbook.pdf)
