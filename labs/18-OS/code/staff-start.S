#include "rpi-asm.h"

@ linker hack so we can force this file in the first portion of 
@ the linked binary.
.section ".text.boot"

@ this is where we start.  if you look in any '.list' file, you 
@ better see code at the first addresses (in our case: starting
@ at 0x8000).
.globl _start
_start:

    @ force the mode to be SUPER.
    mov r0,  #SUPER_MODE
    orr r0,r0,#(1<<7)    @ disable interrupts.
    msr cpsr, r0
    prefetch_flush(r1)

    mov sp, #STACK_ADDR @ initialize stack pointer
    mov fp, #0          @ clear frame pointer reg.  don't think needed.
    @ bl notmain        @ we could jump right to notmain
    bl _cstart          @ call our code to do initialization.
    bl rpi_reboot       @ if they return just reboot.

@ keep put32/get32 here so that they are in the same location in every binary.
@ if you want to override them easily, can re-define the name in rpi-asm.h
@
@ we write them as assembly routines to make absolutely sure the compiler
@ cannot optimize them (reorder, eliminate, cache, etc.)

@
@ put32(addr,val).  c-code equivalant:
@   - put32(addr, val) { *(uint32_t*)addr = val; }
@
@ store value <val> (passed in second argument register r1) 
@ into memory address <addr> (passed in first argument 
@ register, r0).
@
.globl put32
.globl PUT32
put32:
PUT32:
    str r1,[r0]     @ store r1 into address held in r0
    bx lr           @ return

@ get32(addr)
@
@ c-code equivalent: 
@   - get32(addr) { return *(uint32_t*)addr; }
@
@ load value of <addr> passed in first argument register (r0).
@
.globl get32
.globl GET32
get32:
GET32:
    ldr r0,[r0]     @ load address held in r0 into r0
    bx lr           @ return


@ jump to the address in r0.  we don't mess w/ link register
@ so called routine will return to our caller.
.globl BRANCHTO
BRANCHTO:
    bx r0

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ the routines below should give the same hash for everyone.
@   1. this file is linked first so will be at the same location
@      in every binary.
@   2. everyone's assembler should convert this assembly code to 
@      the same machine code  (not true for C code).
@   3. therefore: the same code will be at the same location for everyone.
@   4. therefore: the single step hashes will be the same:
@           - same instructions;
@           - at same location;
@           - run on same initial state;
@      = same result.
@
@  this determinism let's us cross check our results across
@  everyone(!) and across time(!).

@ has the defines for each system call.
#include "eqx-syscalls.h"

@ first trivial test:
@  - one nop
@  - load integer for EXIT into r0
@  - do system call.
@ do this first to see that the code works.
MK_FN(nop_1)
    nop
    mov r0, #EQX_SYS_EXIT
    swi 1

MK_FN(nop_10)
    nop @ 1
    nop @ 2
    nop @ 3
    nop @ 4
    nop @ 5
    nop @ 6
    nop @ 7
    nop @ 8
    nop @ 9
    nop @ 10
    @ note this will jump to exit b/c of how we
    @ setup the thread code.
    bx lr   
    asm_not_reached()

@ move a constant = to each register
@ into each general purpose register.
@   r0 = 0
@   r1 = 1,
@   r12 = 12
MK_FN(mov_ident)
    mov r0, #0
    mov r1, #1
    mov r2, #2
    mov r3, #3
    mov r4, #4
    mov r5, #5
    mov r6, #6
    mov r7, #7
    mov r8, #8
    mov r9, #9
    mov r10, #10
    mov r11, #11
    mov r12, #12
    @ wait: why did we do this?  this is stack.
    @ shouldn't matter?
    @ mov r13, #13
    bx lr
    asm_not_reached()

@ simple test:
@   exit with code=1
MK_FN(small1)
    mov r1, #1
    mov r0, #EQX_SYS_EXIT
    swi 1

@ simple test 
@   exit with code=2
MK_FN(small2)
    mov r1, #2
    mov r0, #EQX_SYS_EXIT
    swi 1

@ do exit system call.   we have a seperate 
@ routine so we can just assign it to the LR in 
@ the initial thread registers.
MK_FN(sys_equiv_exit)
    mov r0, #EQX_SYS_EXIT
    swi 1

@ do <putc> system call.  
MK_FN(sys_equiv_putc)
    push {lr}
    mov r1, r0
    mov r0, #EQX_SYS_PUTC
    swi 1
    pop {lr}
    bx lr


@ user-level trampoline to call system call.
@
@ save lr b/c hardware will trash it (since we are at SUPER initially)
MK_FN(syscall_invoke_asm)
    push {lr}   @ we won't need these at user level.
    swi 0
    pop {lr}    @ we won't need these at user level.
    bx lr

