#include "rpi-asm.h"

@ we only expect the interrupt to get forwarded.  the rest
@ call default trampolines in:
@        <libpi/staff-src/unhandled-exception.S>
.align 5
.globl interrupt_vec
interrupt_vec:
  ldr pc, =unhandled_reset
  ldr pc, =unhandled_undefined_instruction
  ldr pc, =unhandled_swi
  ldr pc, =unhandled_prefetch_abort
  ldr pc, =unhandled_data_abort
  ldr pc, =interrupt_asm

@ only <interrupt_vector> should run since we only enable general interrupts
interrupt_asm:
  mov sp, #INT_STACK_ADDR   
  sub   lr, lr, #4
  push  {r0-r12,lr}         @ XXX: pushing too many registers: only need caller
  mov   r0, lr              @ Pass old pc
  bl    interrupt_vector    @ C function
  pop   {r0-r12,lr}         @ pop integer registers
  movs    pc, lr        @ moves the link register into the pc and implicitly
                        @ loads the PC with the result, then copies the
                        @ SPSR to the CPSR.
