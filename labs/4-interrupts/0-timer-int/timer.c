/*
 * PLEASE MAKE A COPY OF THIS DIRECTORY BEFORE MODIFYING.  We
 * might still make changes before class.
 *
 * PLEASE MAKE A COPY OF THIS DIRECTORY BEFORE MODIFYING.  We
 * might still make changes before class.
 *
 * PLEASE MAKE A COPY OF THIS DIRECTORY BEFORE MODIFYING.  We
 * might still make changes before class.
 *
 * PLEASE MAKE A COPY OF THIS DIRECTORY BEFORE MODIFYING.  We
 * might still make changes before class.
 *
 * Complete working timer-interrupt code.
 *  - the associated assembly code is in <interrupts-asm.S>
 *  - Search for "Q:" to find the various questions.   Change 
 *    the code to answer.
 *
 * You should run it and play around with changing values.
 */
#include "rpi.h"

//*******************************************************
// interrupt initialization and support code.

// timer interrupt number defined in 
//     bcm 113: periph interrupts table.
// used in:
//   - Basic pending (p113);
//   - Basic interrupt enable (p 117) 
//   - Basic interrupt disable (p118)
#define ARM_Timer_IRQ   (1<<0)

// registers for ARM interrupt control
// bcm2835, p112   [starts at 0x2000b200]
enum {
    IRQ_Base            = 0x2000b200,
    IRQ_basic_pending   = IRQ_Base+0x00,    // 0x200
    IRQ_pending_1       = IRQ_Base+0x04,    // 0x204
    IRQ_pending_2       = IRQ_Base+0x08,    // 0x208
    FIQ_control         = IRQ_Base+0x0c,    // 0x20c
    Enable_IRQs_1       = IRQ_Base+0x10,    // 0x210
    Enable_IRQs_2       = IRQ_Base+0x14,    // 0x214
    Enable_Basic_IRQs   = IRQ_Base+0x18,    // 0x218
    Disable_IRQs_1      = IRQ_Base+0x1c,    // 0x21c
    Disable_IRQs_2      = IRQ_Base+0x20,    // 0x220
    Disable_Basic_IRQs  = IRQ_Base+0x24,    // 0x224
};

// registers for ARM timer
// bcm 14.2 p 196
enum { 
    ARM_Timer_Base      = 0x2000B400,
    ARM_Timer_Load      = ARM_Timer_Base + 0x00, // p196
    ARM_Timer_Value     = ARM_Timer_Base + 0x04, // read-only
    ARM_Timer_Control   = ARM_Timer_Base + 0x08,
    ARM_Timer_IRQ_Clear = ARM_Timer_Base + 0x0c,
    // ...
    ARM_Timer_Reload    = ARM_Timer_Base + 0x18,
    ARM_Timer_PreDiv    = ARM_Timer_Base + 0x1c,
    ARM_Timer_Counter   = ARM_Timer_Base + 0x20,
};

//**************************************************
// one-time initialization of general purpose 
// interrupt state
static void interrupt_init(uint32_t ncycles) {
    // defined in <interrupts-asm.S>
    void disable_interrupts(void);
    void enable_interrupts(void);

    printk("about to install interrupt handlers\n");

    // turn off global interrupts.  (should be off
    // already for today)
    disable_interrupts();

    // put interrupt flags in known state by disabling
    // all interrupt sources (1 = disable).
    //  BCM2835 manual, section 7.5 , 112
    PUT32(Disable_IRQs_1, 0xffffffff);
    PUT32(Disable_IRQs_2, 0xffffffff);
    dev_barrier();

    // Copy interrupt vector table and FIQ handler.
    //   - <_interrupt_table>: start address 
    //   - <_interrupt_table_end>: end address 
    // these are defined as labels in <interrupt-asm.S>
    extern uint32_t _interrupt_table[];
    extern uint32_t _interrupt_table_end[];

    // A2-16: first interrupt code address at <0> (reset)
    uint32_t *dst = (void *)0;

    // copy the handlers to <dst>
    uint32_t *src = _interrupt_table;
    unsigned n = _interrupt_table_end - src;

    // these writes better not migrate!
    gcc_mb();
    for(int i = 0; i < n; i++)
        dst[i] = src[i];
    gcc_mb();

    //**************************************************
    // now that we are sure the global interrupt state is
    // in a known, off state, setup and enable 
    // timer interrupts.
    printk("setting up timer interrupts\n");
    
    // bcm p 116
    // write a 1 to enable the timer inerrupt , 
    // "all other bits are unaffected"
    PUT32(Enable_Basic_IRQs, ARM_Timer_IRQ);

    // dev barrier b/c the ARM timer is a different device
    // than the interrupt controller.
    dev_barrier();

    // system timer interrupt */
    /* Timer frequency = Clk/256 * Load --- so smaller = more frequent. */
    PUT32(ARM_Timer_Load, ncycles);

    // bcm p 197
    enum { 
        // note errata!  not a 23 bit.
        ARM_TIMER_CTRL_32BIT        = ( 1 << 1 ),
        ARM_TIMER_CTRL_PRESCALE_1   = ( 0 << 2 ),
        ARM_TIMER_CTRL_PRESCALE_16  = ( 1 << 2 ),
        ARM_TIMER_CTRL_PRESCALE_256 = ( 2 << 2 ),
        ARM_TIMER_CTRL_INT_ENABLE   = ( 1 << 5 ),
        ARM_TIMER_CTRL_ENABLE       = ( 1 << 7 ),
    };

    // Q: if you change prescale?
    PUT32(ARM_Timer_Control,
            ARM_TIMER_CTRL_32BIT |
            ARM_TIMER_CTRL_ENABLE |
            ARM_TIMER_CTRL_INT_ENABLE |
            ARM_TIMER_CTRL_PRESCALE_16);

    // done modifying timer: do a dev barrier since
    // we don't know what device gets used next.
    dev_barrier();

    //**************************************************
    // it's go time: enable global interrupts and we will 
    // be live.

    printk("gonna enable ints globally!\n");
    // Q: what happens (&why) if you don't do?
    enable_interrupts();
    printk("enabled!\n");
}

// catch if unexpected exceptions occur.
// these are referenced in <interrupt-asm.S>
// none of them should be called for this code.
void fast_interrupt_vector(unsigned pc) {
    panic("unexpected fast interrupt: pc=%x\n", pc);
}
void syscall_vector(unsigned pc) {
    panic("unexpected syscall: pc=%x\n", pc);
}
void reset_vector(unsigned pc) {
    panic("unexpected reset: pc=%x\n", pc);
}
void undefined_instruction_vector(unsigned pc) {
    panic("unexpected undef-inst: pc=%x\n", pc);
}
void prefetch_abort_vector(unsigned pc) {
    panic("unexpected prefetch abort: pc=%x\n", pc);
}
void data_abort_vector(unsigned pc) {
    panic("unexpected data abort: pc=%x\n", pc);
}

//*******************************************************
// timer interrupt handler: this should get called.

// Q: if you make not volatile?
static volatile unsigned cnt, period, period_sum;

// called by <interrupt-asm.S> on each interrupt.
void interrupt_vector(unsigned pc) {
    dev_barrier();
    unsigned pending = GET32(IRQ_basic_pending);

    // if this isn't true, could be a GPU interrupt (as discussed in Broadcom):
    // just return.  [confusing, since we didn't enable!]
    if((pending & ARM_Timer_IRQ) == 0)
        return;

    // Checkoff: add a check to make sure we have a timer interrupt
    // use p 113,114 of broadcom.

    /* 
     * Clear the ARM Timer interrupt - it's the only interrupt we have
     * enabled, so we don't have to work out which interrupt source
     * caused us to interrupt 
     *
     * Q: what happens, exactly, if we delete?
     */
    PUT32(ARM_Timer_IRQ_Clear, 1);

    /*
     * We do not know what the client code was doing: if it was touching a 
     * different device, then the broadcom doc states we need to have a
     * memory barrier.   NOTE: we have largely been living in sin and completely
     * ignoring this requirement for UART.   (And also the GPIO overall.)  
     * This is probably not a good idea and we should be more careful.
     */
    dev_barrier();    
    cnt++;

    // compute time since the last interrupt.
    static unsigned last_clk = 0;
    unsigned clk = timer_get_usec();
    period = last_clk ? clk - last_clk : 0;
    period_sum += period;
    last_clk = clk;

    // Q: what happens (&why) if you uncomment the print statement?
    // printk("In interrupt handler at time: %d\n", clk);
}

// simple driver: initialize and then run. 
void notmain() {
    // Q: if you change 0x100?
    interrupt_init(0x100);




    //**************************************************
    // loop until we get N interrupts, tracking how many
    // times we can iterate.
    unsigned start = timer_get_usec();

    // Q: what happens if you enable cache?  Why are some parts
    // the same, some change?
    //enable_cache(); 	
    unsigned iter = 0, sum = 0;
#   define N 20
    while(cnt < N) {
        // Q: if you comment this out?  why do #'s change?
        printk("iter=%d: cnt = %d, time between interrupts = %d usec (%x)\n", 
                                    iter,cnt, period,period);
        iter++;
    }







    //********************************************************
    // overly complicated calculation of sec/ms/usec breakdown
    // make it easier to tell the overhead of different changes.
    // not really specific to interrupt handling.
    unsigned tot = timer_get_usec() - start,
             tot_sec    = tot / (1000*1000),
             tot_ms     = (tot / 1000) % 1000,
             tot_usec   = (tot % 1000);
    printk("-----------------------------------------\n");
    printk("summary:\n");
    printk("\t%d: total iterations\n", iter);
    printk("\t%d: tot interrupts\n", N);
    printk("\t%d: iterations / interrupt\n", iter/N);
    printk("\t%d: average period\n\n", period_sum/(N-1));
    printk("total execution time: %dsec.%dms.%dusec\n", 
                    tot_sec, tot_ms, tot_usec);
}
