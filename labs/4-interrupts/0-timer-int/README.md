## A complete working ARM1176 timer interrupt example

***before modifyig, make a copy of this directory***

Two files:
  - `timer.c`: all the C code needed to setup hardware state
     so we can get timer interrupts.  contains a simple 
     interrupt handler.
  - `interrupts-asm.S`: the assembly code needed to support
    interrupts.

In addition, we make local copies of two `libpi` files 
so that you can make modifications to them if desired:
  - `libpi/memmap`: linker script used to layout the binary.
  - `libpi/staff-start.S`: the assembly used to start a binary.
