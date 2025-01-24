#include "rpi.h"

#include "uart.h"

// 2.1, p8
static volatile unsigned 
#	define ENABLE_UART 0x1
	*const aux_enables = (void*)0x20215004;

#if 0
enum {
    UART_IO = UART_BASE + 4,
    UART_IER = UART_IO+4,
    UART_IIR = UART_IER+4,
};
#endif

/*
 * I've used two styles of device memory specification:
 *  1. when we treat the fields as opaque units, just use enums so they can't be 
 *   dereferenced directly.
 *  2. when there are a lot of internal bit-level fields that we need, use bitfields.
 *    this allows us to have the compiler generate the code to read-modify-write them
 *    rather than us defining (or mis-defining) many different flags, masks, etc.
 *
 * since we are not currently R-M-W the internals, use enums.  
 */

#if 0
// 2.1, p8
// XXX: go through and do the bitfields for these.
struct aux_periphs {
    volatile unsigned 
        /* <aux_mu_> regs */
        io,		// p11
        ier,

#       define CLEAR_TX_FIFO 	(1 << 1)
#       define CLEAR_RX_FIFO 	(1 << 2)
#       define CLEAR_FIFOs 	(CLEAR_TX_FIFO|CLEAR_RX_FIFO)
        // dwelch does not write the low bit?
#       define IIR_RESET    	((0b11 << 6) | 1)
        iir,

        lcr,
        mcr,
        lsr,
        msr,
        scratch, 

#       define RX_ENABLE (1 << 0)
#       define TX_ENABLE (1 << 1)
        cntl,

        stat,
        baud; 	
};
#endif

static struct aux_periphs * const uart = (void*)0x20215040;

struct aux_periphs *uart_get(void) { return uart; }

// should build this in.
static void or_in32(volatile void *addr, unsigned val) {
    put32(addr, get32(addr) | val); 
}

static void rmw32(volatile void *addr, unsigned val, unsigned bit_off, unsigned size) {
    assert(bit_off < 32);
    assert(size < 32);
    // make sure field fits
    assert((val >> size) == 0);

    // set <size> bits to 1.
    unsigned mask = (1<<(size+1)) - 1;
    // shift the mask to position.
    mask = mask << bit_off;
    // complement so that everything else is 1.
    mask = ~mask;

    // double check.
    assert(((val << bit_off) & mask) == 0);

    put32(addr, get32(addr) | val); 
}

/*
 * p10:
 *	- 7 or 8 bit op
 *	- 1 start and 1 stop bit.
 *	- no parity
 *	- no break detection.
 * A bunch of miniUART stuff is wrong, esp the data size field in LCR:
 * 	https://elinux.org/BCM2835_datasheet_errata
 */
void uart_init(void) {
    // p10 claims: gpio pins should be setup first before enabling
    // the uart... if rx is low (b/c not set up yet) that will be
    // seen as a start bit and the uart will receive 0x0 characters.
    //
    // However, dwelch67 does these last, then the single put32 to
    // enable rx,tx.
    gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5);
    gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);

    dev_barrier();

    // p9/10: bit(0) = miniuart enable.  must do first, or nothing
    // happens (can't even read uart regs).  the worrying statement
    // from the doc is that the uart will "immediately start receiving 
	// data" (which seems bad).

    or_in32(aux_enables, ENABLE_UART);

    dev_barrier();

    // p10 after reset: baudrate = 0 and the system clock = 250Mhz.

    // I think we ignore on init:
    // 	- io (p11)
    // 	- lsr (p15)
    // 	- msr? (p15)
    // 	- stat? p18
    // 	- scratch?

    // to be safe clear rx,tx (note: dwelch67 had it below ier).
    put32(&uart->cntl, 0);

    // p12: reset to ensure we disable interrupts.
    put32(&uart->ier, 0); 			// errata: table header wrong.

    // p14: set data size to 8bit (0 for everything else = reset).
    // note: the broadcom doc is wrong, the errata states: 
    // 8 bit is 0b11, not 0b1
    put32(&uart->lcr, 0b11);	

    // p14: clear it.
    put32(&uart->mcr, 0);

    // p13 clear FIFO, put reset values for the rest.
    put32(&uart->iir, IIR_RESET | CLEAR_FIFOs);// same errata.

    // p11 gives baudrate as:
    // 	baudrate = system_clk / (8 * (baud_reg + 1))
    // so, solve for baud_reg:
    //	baud_reg + 1 = system_clk / (8 * baudrate)
    // 	baud_reg = system_clk / (8*baudrate) - 1
    //
    // for 115200 then:
    // 	system clock rate = 250Mhz = 250,000,000.
    //
    // 	baud_reg = 250,000,000/(115200*8) - 1 = 270
    put32(&uart->baud, 270);

    // NOTE: dwelch had the gpio enabled here, right before writing
    // 0b11 to cntl.

    put32(&uart->cntl, RX_ENABLE | TX_ENABLE);
    dev_barrier();
}

// Note: LSR has identical abilities, dwelch67 uses that.
//
// XXX: should assert that both are equal.
int uart_can_getc(void) {
#if 0
    // bit 0 if holds at least 1 byte.
    return (get32(&uart->lsr) & 1) != 0;
#else
    // p18: bit(0): if bit=1, the rx FIFO contains at least one byte.
    return (get32(&uart->stat) & 0b01) != 0;
#endif
}
int uart_can_putc(void) {
// XXX: should assert that both are equal.
#if 0
    // p15 (1<<5) if can accept 1 byte.
    return (get32(&uart->lsr) & (1<<5)) != 0;
#else

    // p18: bit(1): if bit=1, the tx FIFO can accept at least one byte.
    return (get32(&uart->stat) & 0b10) != 0;
#endif
}

// p11: io reg is dual purpose: write to bits[0..7] put byte on fifo,
// reads pull a byte off.

int uart_getc(void) {
    while(!uart_can_getc())
        ;
    return get32(&uart->io) & 0xff;
}
void uart_putc(unsigned c) {
    while(!uart_can_putc())
        ;
    put32(&uart->io, c & 0xff);
}
