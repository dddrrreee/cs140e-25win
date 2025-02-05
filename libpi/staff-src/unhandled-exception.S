@ default unhandled exception vectors: 
@  - load stack pointer.
@  - compute LR
@  - call default C code (which will just panic)
#include "rpi-asm.h"

@ save all the stuff we need, then call the given routine.
#define TRAMPOLINE(fn)              \
  mov sp, #INT_STACK_ADDR;          \
  push  {r0-r12,lr};                \
  mov   r0, lr;                     \
  bl    fn;                         \
  pop   {r0-r12,lr};                \
  movs    pc, lr


// keep the name of the trampoline private.
#define MK_TRAMPOLINE(name, offset, fn)     \
.weak name;                                 \
name:                                       \
    sub   lr, lr, # (offset);               \
    TRAMPOLINE(fn)

@ call the default handlers and blow up.  override if you
@ want to do something fancier.
MK_TRAMPOLINE(unhandled_reset,            4, reset_vector)
MK_TRAMPOLINE(unhandled_undefined_instruction,            4, undefined_instruction_vector)
MK_TRAMPOLINE(unhandled_prefetch_abort,   4, prefetch_abort_vector)
MK_TRAMPOLINE(unhandled_data_abort,       8, data_abort_vector)
MK_TRAMPOLINE(unhandled_interrupt,        4, int_vector)
MK_TRAMPOLINE(unhandled_swi,              4, syscall_vector)
MK_TRAMPOLINE(unhandled_fiq,              4, fiq_vector)

