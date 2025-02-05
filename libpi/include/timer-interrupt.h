/*
 * Complete working timer-interrupt code.
 *  - the associated assembly code is in <interrupts-asm.S>
 *  - Search for "Q:" to find the various questions.   Change 
 *    the code to answer.
 *
 * You should run it and play around with changing values.
 */
#include "rpi.h"
#include "rpi-interrupts.h"

#if 0
enum {
    IRQ_Base            = 0x2000b200,
    IRQ_basic_pending   = IRQ_Base+0x00,    // 0x200
    IRQ_pending_1       = IRQ_Base+0x04,    // 0x204
    IRQ_pending_2       = IRQ_Base+0x08,    // 0x208
    IRQ_FIQ_control     = IRQ_Base+0x0c,    // 0x20c
    IRQ_Enable_1        = IRQ_Base+0x10,    // 0x210
    IRQ_Enable_2        = IRQ_Base+0x14,    // 0x214
    IRQ_Enable_Basic    = IRQ_Base+0x18,    // 0x218
    IRQ_Disable_1       = IRQ_Base+0x1c,    // 0x21c
    IRQ_Disable_2       = IRQ_Base+0x20,    // 0x220
    IRQ_Disable_Basic   = IRQ_Base+0x24,    // 0x224
};
#endif

// registers for ARM timer
// bcm 14.2 p 196
enum { 
    ARM_Timer_Base      = 0x2000B400,
    ARM_Timer_Load      = ARM_Timer_Base + 0x00, // p196
    ARM_Timer_Value     = ARM_Timer_Base + 0x04, // read-only
    ARM_Timer_Control   = ARM_Timer_Base + 0x08,

    ARM_Timer_IRQ_Clear = ARM_Timer_Base + 0x0c,

    // Errata fo p198: 
    // neither are register 0x40C raw is 0x410, masked is 0x414
    ARM_Timer_IRQ_Raw   = ARM_Timer_Base + 0x10,
    ARM_Timer_IRQ_Masked   = ARM_Timer_Base + 0x14,

    ARM_Timer_Reload    = ARM_Timer_Base + 0x18,
    ARM_Timer_PreDiv    = ARM_Timer_Base + 0x1c,
    ARM_Timer_Counter   = ARM_Timer_Base + 0x20,
};

// initialize timer interrupts.
// <prescale> can be 1, 16, 256. see the timer value.
// NOTE: a better interface = specify the timer period.
// worth doing as an extension!
static inline
void timer_init(uint32_t prescale, uint32_t ncycles) {
    //**************************************************
    // now that we are sure the global interrupt state is
    // in a known, off state, setup and enable 
    // timer interrupts.
    printk("setting up timer interrupts\n");

    // assume we don't know what was happening before.
    dev_barrier();
    
    // bcm p 116
    // write a 1 to enable the timer inerrupt , 
    // "all other bits are unaffected"
    PUT32(IRQ_Enable_Basic, ARM_Timer_IRQ);

    // dev barrier b/c the ARM timer is a different device
    // than the interrupt controller.
    dev_barrier();

    // Timer frequency = Clk/256 * Load 
    //   - so smaller <Load> = = more frequent.
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

    uint32_t v = 0;
    switch(prescale) {
    case 1: v = ARM_TIMER_CTRL_PRESCALE_1; break;
    case 16: v = ARM_TIMER_CTRL_PRESCALE_16; break;
    case 256: v = ARM_TIMER_CTRL_PRESCALE_256; break;
    default: panic("illegal prescale=%d\n", prescale);
    }

    // Q: if you change prescale?
    PUT32(ARM_Timer_Control,
            ARM_TIMER_CTRL_32BIT |
            ARM_TIMER_CTRL_ENABLE |
            ARM_TIMER_CTRL_INT_ENABLE |
            v);

    // done modifying timer: do a dev barrier since
    // we don't know what device gets used next.
    dev_barrier();
}

