/*
 * implement these.  ordered in difficulty.
 */
#include "rpi-asm.h"

.data

@ return the current stack pointer.
MK_FN(rpi_get_sp)
	bx lr

@empty stub routines.  use these, or make your own.
MK_FN(rpi_cswitch_old2)
    stmia r0, {r4-r12,r14}
    str   sp, [r0, #10*4]

    mov r4, #4
    mov r5, #5
    mov r6, #6
    mov r7, #7
    mov r8, #8
    mov r9, #9
    mov r10, #10
    mov r11, #11
    mov r12, #12
    mov lr, #14


    ldmia r1, {r4-r12,r14}
    ldr   sp, [r1, #10*4]
    bx lr

MK_FN(rpi_cswitch)
    push {r4,r5,r6,r7,r8,r9,r10,r11,lr}
    str sp, [r0,#0]

    mov sp, r1
    @ mov r0, sp
    @ bl rpi_print_regs
    pop {r4,r5,r6,r7,r8,r9,r10,r11,lr}
    bx lr

MK_FN(cswitch)
    push {r4,r5,r6,r7,r8,r9,r10,r11,lr}
    str sp, [r0,#0]

    ldr sp, [r1,#0]
    @ mov r0, sp
    @ bl rpi_print_regs
    pop {r4,r5,r6,r7,r8,r9,r10,r11,lr}
    bx lr


MK_FN(rpi_cswitch_old)
    str   r4,  [r0, #0]
    str   r5,  [r0, #4]
    str   r6,  [r0, #8]
    str   r7,  [r0, #12]
    str   r8,  [r0, #16]
    str   r9,  [r0, #20]
    str   r10, [r0, #24]
    str   r11, [r0, #28]
    str   r12, [r0, #32]
    str   r12, [r0, #32]
    @ sp
    str   r13, [r0, #36]
    @ lr
    str   r14, [r0, #40]

    ldr   r4,  [r1, #0]
    ldr   r5,  [r1, #4]
    ldr   r6,  [r1, #8]
    ldr   r7,  [r1, #12]
    ldr   r8,  [r1, #16]
    ldr   r9,  [r1, #20]
    ldr   r10, [r1, #24]
    ldr   r11, [r1, #28]
    ldr   r12, [r1, #32]
    ldr   r12, [r1, #32]
    @ sp
    ldr   r13, [r1, #36]
    @ lr
    ldr   r14, [r1, #40]
    bx lr


@ [Make sure you can answer: why do we need to do this?]
@
@ use this to setup each thread for the first time.
@ setup the stack so that when cswitch runs it will:
@	- load address of <rpi_init_trampoline> into LR
@	- <code> into r1, 
@	- <arg> into r0
@ 
MK_FN(rpi_init_trampoline_old)
    mov  r0, r4
    mov  r2, sp
    blx  r5

    mov r0, #0
    bl rpi_exit

MK_FN(rpi_init_trampoline)
    mov  r0, r4
    blx  r5

    mov r0, #0
    bl rpi_exit

MK_FN(using_staff_rpi_thread_asm)
    asm_not_reached()
