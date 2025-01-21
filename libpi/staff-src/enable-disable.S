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
.globl enable_interrupts
enable_interrupts:
    mrs r0,cpsr         @ move process status register (PSR) to r0
    bic r0,r0,#(1<<7)   @ clear 7th bit.
    msr cpsr_c,r0       @ move r0 back to PSR
    bx lr               @ return.

.globl disable_interrupts
disable_interrupts:
    mrs r0,cpsr
    orr r0,r0,#(1<<7)          @ set 7th bit: or in 0b100 0000
    msr cpsr_c,r0
    bx lr
