/*
 * interrupt-asm.s
 *
 * Code for interrupt handling.  Refer to armisa.pdf in docs/ for what
 * the opcodes mean.
 */
#include "rpi-asm.h"

.data

.globl _interrupt_table
.globl _interrupt_table_data
.globl _interrupt_table_end
_interrupt_table:
  ldr pc, _reset_asm                    @ 0
  ldr pc, _undefined_instruction_asm    @ 1
  ldr pc, _software_interrupt_asm       @ 2
  ldr pc, _prefetch_abort_asm           @ 3
  ldr pc, _data_abort_asm               @ 4
  ldr pc, _reset_asm                    @ 5
  ldr pc, _interrupt_asm                @ 6
fast_interrupt_asm:
  sub   lr, lr, #4 @First instr of FIQ handler
  push  {lr}
  push  {r0-r12}
  mov   r0, lr              @ Pass old pc
  bl    fast_interrupt_vector    @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

_interrupt_table_data:
_reset_asm:                   .word reset_asm
_undefined_instruction_asm:   .word undefined_instruction_asm
_software_interrupt_asm:      .word software_interrupt_asm
_prefetch_abort_asm:          .word prefetch_abort_asm
_data_abort_asm:              .word data_abort_asm
_interrupt_asm:               .word interrupt_asm
_interrupt_table_end:

@ only handler that should run since we only enable general interrupts
MK_FN(interrupt_asm)
  mov sp, #INT_STACK_ADDR  @ i believe we have 512mb - 16mb, so this should be safe
  sub   lr, lr, #4

  push  {r0-r12,lr}         @ XXX: pushing too many registers: only need caller
  @ vpush {s0-s15}	        @ uncomment if want to save caller-saved fp regs

  mov   r0, lr              @ Pass old pc
  bl    interrupt_vector    @ C function

  @ vpop {s0-s15}           @ pop caller saved fp regs
  pop   {r0-r12,lr} 	    @ pop integer registers

  @ return from interrupt handler: will re-enable general ints.
  movs    pc, lr        @ moves the link register into the pc and implicitly
                        @ loads the PC with the result, then copies the 
                        @ SPSR to the CPSR.


#define TRAMPOLINE(fn)              \
  mov sp, #INT_STACK_ADDR;          \
  push  {r0-r12,lr};                \
  mov   r0, lr;                     \
  bl    fn;                         \
  pop   {r0-r12,lr};                \
  movs    pc, lr        

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ we don't generate any of these, will just panic and halt.
@
MK_FN(reset_asm)
    sub   lr, lr, #4
    TRAMPOLINE(reset_vector)
MK_FN(undefined_instruction_asm)
    sub   lr, lr, #4
    TRAMPOLINE(undefined_instruction_vector)
MK_FN(prefetch_abort_asm)
    sub   lr, lr, #4
    TRAMPOLINE(prefetch_abort_vector)
MK_FN(data_abort_asm)
    sub   lr, lr, #8
    TRAMPOLINE(data_abort_vector)
MK_FN(software_interrupt_asm)
  sub   lr, lr, #4
  TRAMPOLINE(syscall_vector)

@software_interrupt_asm:
@    push    {r3-r12,lr}   
@    mov     r3,sp
@    bl      handle_swi_full
@    pop     {r3-r12,lr}     
@    movs    pc, lr

MK_FN(using_staff_interrupts_asm)
    asm_not_reached()
