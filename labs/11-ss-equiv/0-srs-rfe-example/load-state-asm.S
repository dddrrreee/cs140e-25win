
@ hack for testing dumb swi calls.
#include "rpi-asm.h"

MK_FN(check_rfe_asm)
    mov r1, sp
    mov r2, lr
    bl check_rfe
    b impossible


@ use the SRS instruction to save {lr,spsr}
@ of SUPER mode to the memory held in r0.
@
@ we also manually store lr into the memory 
@ pointed to by r1 so we can compare the result.
MK_FN(srs_super)
    @ armv6 a4-174
    str   lr, [r1]    @ save the actual lr so we can compare

    @ srs requires sp as the only register
    mov   r2, sp            @ save sp
    mov   sp, r0            @ set sp to the point passed in r0
    srsea sp, #SUPER_MODE   @ EA suffix = empty ascending stack.
    mov   sp, r2            @ restore sp.
    bx lr

@ restore user state using the RFE instruction.
MK_FN(rfe_user)
    @ we use lr as a spare register so we an restore
    @ everything easily
    mov lr, r0

    @ restore user level sp and lr.
    ldm   lr, {sp,lr}^
    add lr, lr, #8
    @ now return to user -- assumes values go upwards
    rfe lr  @ same as: rfefd lr

    rfefd lr
    rfeea lr
    b impossible
