#ifndef __RPI_INTERRUPT_H__
#define __RPI_INTERRUPT_H__
// defined in <interrupts-asm.S>
void disable_interrupts(void);
void enable_interrupts(void);

//*******************************************************
// interrupt initialization and support code.

// timer interrupt number defined in 
//     bcm 113: periph interrupts table.
// used in:
//   - Basic pending (p113);
//   - Basic interrupt enable (p 117) 
//   - Basic interrupt disable (p118)
#define ARM_Timer_IRQ   (1<<0)

// from the valvers description.

/** @brief Bits in the <IRQ_Enable_Basic> register to enable 
   various interrupts.
   See the BCM2835 ARM Peripherals manual, section 7.5 */
#define RPI_BASIC_ARM_TIMER_IRQ         (1 << 0)
#define RPI_BASIC_ARM_MAILBOX_IRQ       (1 << 1)
#define RPI_BASIC_ARM_DOORBELL_0_IRQ    (1 << 2)
#define RPI_BASIC_ARM_DOORBELL_1_IRQ    (1 << 3)
#define RPI_BASIC_GPU_0_HALTED_IRQ      (1 << 4)
#define RPI_BASIC_GPU_1_HALTED_IRQ      (1 << 5)
#define RPI_BASIC_ACCESS_ERROR_1_IRQ    (1 << 6)
#define RPI_BASIC_ACCESS_ERROR_0_IRQ    (1 << 7)


// registers for ARM interrupt control
// bcm2835, p112   [starts at 0x2000b200]
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

//**************************************************
// one-time initialization of general purpose 
// interrupt state.
static void inline interrupt_init_v(void *start, void *end) {
    printk("about to install interrupt handlers\n");
    assert(start < end);

    // turn off global interrupts.  (should be off
    // already for today)
    disable_interrupts();

    // put interrupt flags in known state by disabling
    // all interrupt sources (1 = disable).
    //  BCM2835 manual, section 7.5 , 112
    PUT32(IRQ_Disable_1, 0xffffffff);
    PUT32(IRQ_Disable_2, 0xffffffff);
    dev_barrier();

    // A2-16: first interrupt code address at <0> (reset)
    uint32_t *dst = (void *)0;

    // copy the handlers to <dst>
    uint32_t *src = start;
    unsigned n = (uint32_t*)end - src;

    // these writes better not migrate!
    gcc_mb();
    for(int i = 0; i < n; i++)
        dst[i] = src[i];
    gcc_mb();
}

static void inline interrupt_init(void) {
    // Copy interrupt vector table and FIQ handler.
    //   - <_interrupt_table>: start address 
    //   - <_interrupt_table_end>: end address 
    // these are defined as labels in <interrupt-asm.S>
    extern uint32_t _interrupt_table[];
    extern uint32_t _interrupt_table_end[];
    interrupt_init_v(_interrupt_table, _interrupt_table_end);
}


// reset interrupt vector to <vec>.
void * int_vec_reset(void *vec);

// initialize interrupts and set interrupt vector to <vec>.
void int_vec_init(void *v);

#endif
