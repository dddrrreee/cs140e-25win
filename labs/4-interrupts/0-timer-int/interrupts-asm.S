/*
 * Assembly code for interrupt handling.  
 */
#include "rpi-asm.h"

/*
 * Enable/disable interrupts.
 *
 * CPSR = current program status register
 *        upper bits are different carry flags.
 *        lower 8:
 *           7 6 5 4 3 2 1 0
 *          +-+-+-+---------+
 *          |I|F|T|   Mode  |
 *          +-+-+-+---------+
 *
 *  I : disables IRQ when = 1.
 *  F : disables FIQ when = 1.
 *  T : = 0 indicates ARM execution, = 1 is thumb execution.
 *      Mode = current mode.
 */

@ enable system interrupts by modifying cpsr.
@    note: should make a version that returns previous state.
.globl enable_interrupts
enable_interrupts:
    mrs r0,cpsr         @ move process status register (PSR) to r0
    bic r0,r0,#(1<<7)	@ clear 7th bit.
    msr cpsr_c,r0		@ move r0 back to PSR
    bx lr		        @ return.

@ disable them
.globl disable_interrupts
disable_interrupts:
    mrs r0,cpsr		       
    orr r0,r0,#(1<<7)	       @ set 7th bit: or in 0b100 0000
    msr cpsr_c,r0
    bx lr

@ the interrupt table that we copy to 0x0.
@ note: *it must be position independent since
@ we copy it!*
.globl _interrupt_table
.globl _interrupt_table_end
_interrupt_table:
  @ Q: why can we copy these ldr jumps and have
  @ them work the same?
  ldr pc, _reset_asm
  ldr pc, _undefined_instruction_asm
  ldr pc, _software_interrupt_asm
  ldr pc, _prefetch_abort_asm
  ldr pc, _data_abort_asm
  ldr pc, _reset_asm
  ldr pc, _interrupt_asm
fast_interrupt_asm:
  sub   lr, lr, #4 @First instr of FIQ handler
  push  {lr}
  push  {r0-r12}
  mov   r0, lr              @ Pass old pc
  bl    fast_interrupt_vector    @ C function
  pop   {r0-r12}
  ldm   sp!, {pc}^

_reset_asm:                   .word reset_asm
_undefined_instruction_asm:   .word undefined_instruction_asm
_software_interrupt_asm:      .word software_interrupt_asm
_prefetch_abort_asm:          .word prefetch_abort_asm
_data_abort_asm:              .word data_abort_asm
_interrupt_asm:               .word interrupt_asm
_interrupt_table_end:   @ end of the table.

@ only handler that should run since we only enable general interrupts
interrupt_asm:
  @ <INT_STACK_ADDR> is a physical address we reserve.
  mov sp, #INT_STACK_ADDR   
  sub   lr, lr, #4

  push  {r0-r12,lr}         @ XXX: pushing too many registers: only need caller
  @ vpush {s0-s15}	        @ uncomment if want to save caller-saved fp regs

  mov   r0, lr              @ Pass old pc as arg 0
  bl    interrupt_vector    @ C function: expects C calling conventions.

  @ vpop {s0-s15}           @ pop caller saved fp regs
  pop   {r0-r12,lr} 	    @ pop integer registers

  @ return from interrupt handler: will re-enable general ints.
  @ Q: what happens if you do "mov" instead?
  movs    pc, lr        @ moves the link register into the pc and implicitly
                        @ loads the PC with the result, then copies the 
                        @ SPSR to the CPSR.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ we don't generate any of these, will just panic and halt.
@
@ you can override these!
reset_asm:
  sub   lr, lr, #4
  bl    reset_vector
undefined_instruction_asm:
  sub   lr, lr, #4
  bl    undefined_instruction_vector
software_interrupt_asm:
  sub   lr, lr, #4
  bl    syscall_vector
prefetch_abort_asm:
  sub   lr, lr, #4
  bl    prefetch_abort_vector
data_abort_asm:
  sub   lr, lr, #4
  bl    data_abort_vector

@ simple demonstration of how to switch modes to get the
@ banked lr.
#define SUPER_MODE      0b10011
get_super_lr:
.globl get_super_lr
    mrs r1, cpsr
    msr cpsr_c, #SUPER_MODE
    mov r0, lr
    msr cpsr_c, r1
    bx lr
