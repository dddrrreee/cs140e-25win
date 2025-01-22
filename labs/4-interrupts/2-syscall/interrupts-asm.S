/* Assembly code for interrupt handling.  */
#include "rpi-asm.h"

@ the interrupt table that we copy to 0x0.
@   - start = <_interrupt_table>
@   - end = <_interrupt_table_end>
@   - look at the disassembly
@
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

@ only handler that should run since we 
@ only enable general interrupts
interrupt_asm:
  @ NOTE:
  @  - each mode has its own <sp> that persists when
  @    we switch out of the mode (i.e., will be the same
  @    when switch back).
  @  - <INT_STACK_ADDR> is a physical address we reserve 
  @   for exception stacks today.  we don't do recursive
  @   exception/interupts so one stack is enough.
  mov sp, #INT_STACK_ADDR   
  sub   lr, lr, #4

  push  {r0-r12,lr}         @ XXX: pushing too many 
                            @ registers: only need caller
                            @ saved.

  mov   r0, lr              @ Pass old pc as arg 0
  bl    interrupt_vector    @ C function: expects C 
                            @ calling conventions.

  pop   {r0-r12,lr} 	    @ pop integer registers
                            @ this MUST MATCH the push.
                            @ very common mistake.

  @ return from interrupt handler: will re-enable general ints.
  @ Q: what happens if you do "mov" instead?
  @ Q: what other instructions could we use?
  movs    pc, lr        @ 1: moves <spsr> into <cpsr> 
                        @ 2. moves <lr> into the <pc> of that
                        @    mode.


@ change this to call your system call handler.
software_interrupt_asm:
    @ runs at system level (p.a2-5): assumes we have a sp
    @
    @ you're going to call:
    @    int syscall_vector(unsigned pc, uint32_t r0) {
    @
    @   1 save regs as with interrupt vector
    @   2 figure out the lr offset you need to correct.
    @   3 mov the original r0 into r1 (so it's the second
    @     parameter to <syscall_vector>)
    @   4 mov the pointer to the syscall inst into r0
    @     (so its the first parameter to <syscall_vector>)
    @   5 call ,syscall_vector>
    @   6 restore regs: must be identical to what got pushed 
    @     at step (1)
    @   - return from the exception (look at interrupt_asm)
    asm_not_implemented()
    movs pc, lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ currently we don't use any of these, so just panic and
@ and halt.  should not occur!

reset_asm:
    @ Q: if we delete this instruction what happens?
    mov sp, #INT_STACK_ADDR   
    @ Q: is <4> correct?  how to tell?
    sub   r0, lr, #4
    bl    reset_vector

@ note: we compile this <.S> file with gcc 
@ so that we can use C macro's to clean up.
@ note the use of semi-colons!
#define unexpected(fn, offset)      \
    mov sp, #INT_STACK_ADDR;        \
    sub   r0, lr, #(offset);        \
    bl    fn

@ Q: what are the right offsets for the following?
undefined_instruction_asm:
    unexpected(undefined_instruction_vector,4)
prefetch_abort_asm:
    unexpected(prefetch_abort_vector,4)
data_abort_asm:
    unexpected(data_abort_vector,4)

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ simple demonstration of how to switch modes to get the
@ banked lr.
@
@ note from the documents that SYSTEM has the same 
@ registers as USER.  but, unlike user you can switch
@ back from it.
#define SUPER_MODE      0b10011
get_super_lr:
.globl get_super_lr
    mrs r1, cpsr
    msr cpsr_c, #SUPER_MODE
    mov r0, lr
    msr cpsr_c, r1
    bx lr
