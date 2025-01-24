#include "rpi-asm.h"

.data

@ just use the default int stack address.
#define INT_STACK INT_STACK_ADDR

#define not_reached()  mov r0, pc; b asm_not_reached

@ create an interrupt vector named <name>
@ user has to define the different trampolines.
#define MK_INT_VEC(name)    \
    .align 5;               \
    .globl name;            \
    name:                   \
        b reset;            \
        b undef;            \
        b swi_handler;      \
        b prefetch_abort;   \
        b data_abort;       \
        b reset;            \
        b interrupt

@ save all the stuff we need, then call the given routine.
#define TRAMPOLINE(fn)              \
  mov sp, #INT_STACK;               \
  push  {r0-r12,lr};                \
  mov   r0, lr;                     \
  bl    fn;                         \
  pop   {r0-r12,lr};                \
  movs    pc, lr


// keep the name of the trampoline private.
#define MK_TRAMPOLINE(name, offset, fn)  \
    name: sub   lr, lr, # (offset); TRAMPOLINE(fn)


@ call the default handlers and blow up.  override if you
@ want to do something fancier.
MK_TRAMPOLINE(reset,            4, reset_vector)
MK_TRAMPOLINE(undef,            4, undefined_instruction_vector)
MK_TRAMPOLINE(prefetch_abort,   4, prefetch_abort_vector)
MK_TRAMPOLINE(data_abort,       8, data_abort_vector)
MK_TRAMPOLINE(interrupt,        4, int_vector)
MK_TRAMPOLINE(syscall,          4, syscall_vector)

@@ need to make this so you can generate the code at runtime. is a good
@ lab.
@@ not handling system calls.
@swi_handler:
@    @ when we do equiv, need to make sure we restore everything back.
@    @ we over-save/restore and trim after equiv.
@    @ mov sp, #STACK_ADDR
@    
@    @ our stack pointer should be fine, right?
@    mov sp, #STACK_ADDR
@    push {r1-r12,r14}
@    bl do_syscall
@    pop {r1-r12,r14}
@    movs pc, lr

.align 5; 
.globl default_vec_ints
default_vec_ints: 
    b reset
    b undef
    b syscall
    b prefetch_abort
    b data_abort
    b reset
    b interrupt

MK_FN(using_staff_interrupt_vec_asm)
    asm_not_reached()
