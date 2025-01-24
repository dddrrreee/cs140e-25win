/*
 * implement these.  ordered in difficulty.
 */
#include "rpi-asm.h"

######################################################
# called by: 2-where-push-one.c
#
# uint32_t *push_one(uint32_t *ptr, uint32_t val);
#
# 1. use the <push> instruction to push <val>
#    onto the "stack" pointed to by <ptr>.
# 
# 2. after: return the value of <ptr>
#
# NOTE: don't trash the stack pointer <sp>!
MK_FN(push_one)
    asm_todo("implement this!")
    bx lr


#######################################################
# called by: 3-push-order.c
#
# uint32_t *push_two(uint32_t *ptr, uint32_t v1, uint32_t v2);
#
# generalize push_one to push two registers (v1,v2) and
# return the final value of <ptr>.
#
# NOTE: don't trash the stack pointer <sp>!
MK_FN(push_two)
    bx lr


#######################################################
# called by: 5-write-regs.c
#
# void write_regs_to_stack(uint32_t **saved_sp)
#
# routine to:
#  1. check that the context switch save we will do
#     works
#  2. double check the offsets the assembly stores
#     values to.
#
# how:
#   1. save just the callee and lr to the stack.
#   2. write the <sp> value to <saved_sp> when 
#      done.
#   3. call a "no return" routine <after_writing_regs>
#      to print and check the results.
MK_FN(write_regs_to_stack)
    @ set all the callee saved and lr to known values
    @ so we can check what offset they are saved to.
    @
    @ NOTE: this destroys the machine state of the 
    @ caller (e.g., any callee-saved regs) and also
    @ which caller it was (lr is trashed) so we cannot
    @ return from <write_regs_to_stack>
    mov r4, #4
    mov r5, #5
    mov r6, #6
    mov r7, #7
    mov r8, #8
    mov r9, #9
    mov r10, #10
    mov r11, #11
    mov lr, #14

    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    @ 1. push {all the callee + lr} (this is the save
    @    part of your context switch code)
    @ 2. store sp into the address held in r0
    asm_todo(implement this)
    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


    @ now we call the C code to print stuff.  some 
    @ registers are potentially trashed at this point
    @ so the called routine code not return.  use
    @ this trick in your thread code assembly when  
    @ you want to know the value of something.
    mov r0, sp
    bl after_writing_regs
    asm_not_reached()

.weak after_writing_regs
after_writing_regs:
    asm_not_reached()
