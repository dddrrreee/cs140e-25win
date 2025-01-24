@ right now isn't really handling the different cases you'd want:
@ if come from privileged level need to handle differently than USER.
@
@ also: if you just resume after vs <switchto>.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ relabel for students so they can all directly.

#ifndef STAFF_BUILD
#define prefetch_handler_full staff_prefetch_trampoline_full
.global prefetch_handler_full

#define data_abort_full staff_data_abort_trampoline_full
.global data_abort_full

#define syscall_full staff_syscall_trampoline_full
.global syscall_full

#define full_except_ints  staff_full_except_ints
#endif

@ so don't see the code.
.data

#include "rpi-asm.h"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@ these are the non "full except" handlers that work just like all
@ the exceptions/interrupts we've done so far this quarter:
@ just save registers and call the default name.
@
@ included so you have a handy way to see how to use macros in
@ assembly.
@
@ you don't need to modify this (but you can to make everything
@ be a full_except style handler.

@ save all the stuff we need, then call the given routine.
#define TRAMPOLINE(fn)              \
  mov sp, #INT_STACK_ADDR;          \
  push  {r0-r12,lr};                \
  mov   r0, lr;                     \
  bl    fn;                         \
  pop   {r0-r12,lr};                \
  movs    pc, lr

// keep the name of the trampoline private.
#define MK_TRAMPOLINE(name, offset, fn)  \
    name: sub   lr, lr, # (offset); TRAMPOLINE(fn)

@ call the default handlers (if client doesn't define these will just
@ call the libpi versions that panic with an error).
@
@ you can migrate these to full_except if you want!
MK_TRAMPOLINE(reset,            4, reset_vector)
MK_TRAMPOLINE(undef,            4, undefined_instruction_vector)
MK_TRAMPOLINE(prefetch_abort,   4, prefetch_abort_vector)
MK_TRAMPOLINE(interrupt,        4, int_vector)
@ MK_TRAMPOLINE(syscall,          4, syscall_vector)

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@ this is the code you have to write.


#define TRAMPOLINE_REGS(fn)         \
  sub sp, sp, #68;                \
  stm sp, {r0-r14}^;                \
  str lr, [sp, #60];                \
  mov   r2, lr;                 \
  mrs   r1, spsr;                   \
  str r1, [sp, #64];                \
  mov   r0, sp;                     \
  bl    fn;                         \
  asm_not_reached()

prefetch_handler_full:
    mov sp, #INT_STACK_ADDR
    sub   lr, lr, #4
    TRAMPOLINE_REGS(prefetch_abort_full_except)

data_abort_full:
    mov sp, #INT_STACK_ADDR
    sub   lr, lr, #8
    TRAMPOLINE_REGS(data_abort_full_except)

@ assume we have a legit stack: will this work to do the system
@ call and resume over and over even if we are not at 
@ user level? 
syscall_full:
    @ sub lr, lr, #4
    mov sp, #INT_STACK_ADDR
    TRAMPOLINE_REGS(syscall_full_except)

.align 5; 
.globl full_except_ints
full_except_ints: 
    b reset                 @ 0
    b undef                 @ 1 
    b syscall_full          @ 2 
    b prefetch_handler_full @ 3 
    b data_abort_full       @ 4
    b reset                 @ 5
    b interrupt             @ 6
    b bad_exception

bad_exception:
  asm_bad_exception("should not reach here")

@ user level.
.weak syscall_trampoline
MK_FN(syscall_trampoline)
    push {lr}
    swi #1
    pop {lr}
    bx lr
