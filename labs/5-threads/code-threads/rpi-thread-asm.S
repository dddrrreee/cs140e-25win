/*
 * implement these.  ordered in difficulty.
 */
#include "rpi-asm.h"

@ return the current stack pointer.
MK_FN(rpi_get_sp)
    asm_todo("implement this")
    bx lr

@ void rpi_cswitch(uint32_t **old_sp_save, const uint32_t *new_sp);
@   - called to context switch from currently running thread
@     (so must save registers) to next thread (so must load
@     registers).  eg:
@        rpi_cswitch(&cur_th->saved_sp, next_th->saved_sp);
@
@ what to do:
@    1. store all callee + <lr> onto stack.
@    2. store the stack pointer reg into <&cur_th->saved_sp>
@       this causes the most confusion: we have to do this 
@       b/c no one but this code knows where the registers
@       are saved on the stack, so must store this address so 
@       they can be restored later.    single most common
@       confusion, so think about it.
@    3. move <next_th->saved_sp> into the stack pointer reg
@    4. load all the callee + <lr> using the sp.
@    5. return!
MK_FN(rpi_cswitch)
    asm_todo("implement this")
    bx lr

@ [Make sure you can answer: why do we need this
@ weird little routine?]
@
@ use this to setup each thread for the first time.
@ setup the stack so that when cswitch runs it will:
@	- load address of <rpi_init_trampoline> into LR
@	- <code> into r1, 
@	- <arg> into r0
@ 
MK_FN(rpi_init_trampoline)
    asm_todo("implement this")
    bx lr
