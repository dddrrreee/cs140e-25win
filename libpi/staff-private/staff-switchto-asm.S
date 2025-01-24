#ifndef STAFF_BUILD
#define priv_get_sp_lr_asm staff_priv_get_sp_lr_asm
#define switchto_user_asm  staff_switchto_user_asm
#define switchto_priv_asm  staff_switchto_priv_asm
#define cswitchto_priv_asm staff_cswitchto_priv_asm
#define cswitchto_user_asm staff_cswitchto_user_asm
#endif

@ defines exception handlers that load all registers.
#include "rpi-asm.h"

@ so they can't disassemble.

.data

// put these in their own assembly file
@ can't change to user mode directly and set the registers b/c
@ we won't have access to privileged memory holding the registers and
@ we won't be able to run this pc value (since its privileged).
@
@ XXX: is this this the only reason?
@ could we just 1/ put the state on the user stack (or 
@ somewhere) and 2/ alias this code into their address space?
@ not sure.  i think there may be other issues.
MK_FN(switchto_user_asm)
    mov sp, r0

    @ clear the flags in the cpsr: machine will discard anything
    @ we can't write to.

    @ write user level versions of r0..r14
    ldm   sp, {r0-r14}^  
    add  sp, sp, #60
    rfe sp
    asm_not_reached()

@ we can't assume the mode we're jumping to has the right
@ values as "last time" since we've partially executed elsewhere.
MK_FN(switchto_priv_asm)
    @ we switch to the mode we want to go to
    @ and then just load everything.
    ldr   r1, [r0, #64]
    msr   cpsr, r1
    prefetch_flush(r2)
    ldm   r0, {r0-r15}
    asm_not_reached();



@ void mode_get_lr_sp_asm(mode, &sp, &lr)
@
@ note: we use two temporary registers:
@   - r3
@   - r12 
@ both caller saved registers.
MK_FN(priv_get_sp_lr_asm)
    @ save old cpsr
    mrs r3, cpsr

    @ switch modes to get <mode> lr,sp
    msr cpsr_c, r0
    prefetch_flush(r12)     @ not sure we need this.

    str sp, [r1]
    str lr, [r2]

    msr cpsr_c, r3
    prefetch_flush(r12)      @ not sure we need this.
    bx lr

@ cswitch_asm(old_regs, new_regs, fn)
@  - saves all regs into <old_regs>
@  - calls fn(new_regs)
@ register layout is the same as expected by <switchto>.

@ for class: change to being a privileged version, 
@ and just call the right thing.
MK_FN(cswitch_asm)
    // #define CO_SAVE(r)     stm   r, {r4,r5,r6,r7,r8,r9,r10,r11,sp,lr}

    @ save state

    @ zero the caller regs for easier repeatability [actually
    @ this isn't needed?]

    mov r3, #0
    str r3,  [r0, #(0*4)]  @ r0
    str r3,  [r0, #(1*4)]  @ r1
    str r3,  [r0, #(2*4)]  @ r2
    str r3,  [r0, #(3*4)]  @ r3
    str r3,  [r0, #(12*4)]  @ r12

    str r4,  [r0, #(4*4)]  @ r4
    str r5,  [r0, #(5*4)]  @ r5
    str r6,  [r0, #(6*4)]  @ r6
    str r7,  [r0, #(7*4)]  @ r7
    str r8,  [r0, #(8*4)]  @ r8
    str r9,  [r0, #(9*4)]  @ r9
    str r10, [r0, #(10*4)]  @ r10
    str r11, [r0, #(11*4)]  @ r11
    str sp,  [r0, #(13*4)]  @ sp
    str lr,  [r0, #(14*4)]  @ lr

    @ use lr as pc so we resume at the callsite.
    str lr,  [r0, #(15*4)]  @ pc

    @ save cspr
    mrs r3, cpsr;
    str r3, [r0, #(16*4)]; @ cpsr at the end

    mov r0, r1
    blx r2

    @ better not return!
    asm_not_reached();

MK_FN(cswitchto_priv_asm)
    mov r3, #0
    str r3,  [r0, #(0*4)]  @ r0
    str r3,  [r0, #(1*4)]  @ r1
    str r3,  [r0, #(2*4)]  @ r2
    str r3,  [r0, #(3*4)]  @ r3
    str r3,  [r0, #(12*4)]  @ r12

    str r4,  [r0, #(4*4)]  @ r4
    str r5,  [r0, #(5*4)]  @ r5
    str r6,  [r0, #(6*4)]  @ r6
    str r7,  [r0, #(7*4)]  @ r7
    str r8,  [r0, #(8*4)]  @ r8
    str r9,  [r0, #(9*4)]  @ r9
    str r10, [r0, #(10*4)]  @ r10
    str r11, [r0, #(11*4)]  @ r11
    str sp,  [r0, #(13*4)]  @ sp
    str lr,  [r0, #(14*4)]  @ lr

    @ use lr as pc so we resume at the callsite.
    str lr,  [r0, #(15*4)]  @ pc

    @ save cspr
    mrs r3, cpsr;
    str r3, [r0, #(16*4)]; @ cpsr at the end

    @ mov the destination reg into r0
    mov r0, r1
    bl switchto_priv_asm

    @ better not return!
    asm_not_reached();

MK_FN(cswitchto_user_asm)
    mov r3, #0
    str r3,  [r0, #(0*4)]  @ r0
    str r3,  [r0, #(1*4)]  @ r1
    str r3,  [r0, #(2*4)]  @ r2
    str r3,  [r0, #(3*4)]  @ r3
    str r3,  [r0, #(12*4)]  @ r12

    str r4,  [r0, #(4*4)]  @ r4
    str r5,  [r0, #(5*4)]  @ r5
    str r6,  [r0, #(6*4)]  @ r6
    str r7,  [r0, #(7*4)]  @ r7
    str r8,  [r0, #(8*4)]  @ r8
    str r9,  [r0, #(9*4)]  @ r9
    str r10, [r0, #(10*4)]  @ r10
    str r11, [r0, #(11*4)]  @ r11
    str sp,  [r0, #(13*4)]  @ sp
    str lr,  [r0, #(14*4)]  @ lr

    @ use lr as pc so we resume at the callsite.
    str lr,  [r0, #(15*4)]  @ pc

    @ save cspr
    mrs r3, cpsr;
    str r3, [r0, #(16*4)]; @ cpsr at the end

    @ mov the destination reg into r0
    mov r0, r1
    bl switchto_user_asm

    @ better not return!
    asm_not_reached();


MK_FN(switchto_user)
    @ get the original level.
    mrs r0, cpsr

    mov r1, sp
    mov r2, lr
    cps #USER_MODE
    prefetch_flush(r3)

    mov sp, r1
    mov lr, r2
    bx lr;
