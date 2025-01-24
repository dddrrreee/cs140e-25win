#include "rpi.h"
#include "rpi-config.h"
#include "uart.h"

// 2.1, p8
static volatile unsigned 
#	define ENABLE_UART 0x1
	*const aux_enables = (void*)0x20215004;

#ifdef RPI_UNIX
#define get32(x) GET32((uint32_t)(uint64_t)x)
#define put32(x,v) PUT32((uint32_t)(uint64_t)x,v)
#else
#define get32(x) GET32((uint32_t)(uint32_t)x)
#define put32(x,v) PUT32((uint32_t)(uint32_t)x,v)
#endif

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

// struct aux_periphs *uart_get(void) { return uart; }

// should build this in.
static void or_in32(volatile void *addr, unsigned val) {
    put32(addr, get32(addr) | val); 
}
static void and_in32(volatile void *addr, unsigned val) {
    put32(addr, get32(addr) & val); 
}

void uart_disable(void) {
    uart_flush_tx();
    dev_barrier();
    and_in32(aux_enables, ~ENABLE_UART);
    dev_barrier();
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
    dev_barrier();
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
    put32(&uart->iir, CLEAR_FIFOs);// same errata.

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
    uint32_t baud_val = (250 * 1000 * 1000)/(RPI_UART_BAUD*8) - 1;
    put32(&uart->baud, baud_val);

    // NOTE: dwelch had the gpio enabled here, right before writing
    // 0b11 to cntl.

    put32(&uart->cntl, RX_ENABLE | TX_ENABLE);
    dev_barrier();
}

//#define rpi_wait() (void)0

// Note: LSR has identical abilities, dwelch67 uses that.
//
// XXX: should assert that both are equal.
int uart_can_getc(void) {
#if 1
    // bit 0 if holds at least 1 byte.
    return (get32(&uart->lsr) & 1) != 0;
#else
    // p18: bit(0): if bit=1, the rx FIFO contains at least one byte.
    return (get32(&uart->stat) & 0b01) != 0;
#endif
}
int uart_can_putc(void) {
// XXX: should assert that both are equal.
#if 1
    // p15 (1<<5) if can accept 1 byte.
    return (get32(&uart->lsr) & (1<<5)) != 0;
#else

    // p18: bit(1): if bit=1, the tx FIFO can accept at least one byte.
    return (get32(&uart->stat) & 0b10) != 0;
#endif
}
int uart_can_put8(void) {
    return uart_can_putc(); }

// p11: io reg is dual purpose: write to bits[0..7] put byte on fifo,
// reads pull a byte off.

int uart_get8(void) {
    dev_barrier();
    while(!uart_can_getc())
        rpi_wait();
    int c =  get32(&uart->io) & 0xff;
    dev_barrier();
    return c;
}
int uart_put8(uint8_t c) {
    dev_barrier();
    while(!uart_can_putc())
        rpi_wait();
    put32(&uart->io, c & 0xff);
    dev_barrier();
    return 1;
}

int uart_has_data(void) { 
    dev_barrier(); 
    return uart_can_getc(); 
}


int uart_tx_is_empty(void) {
    dev_barrier();

#if 0
    // broadcom p 18: "This bit (9) is set if the transmitter is 
    // idle and the transmit FIFO is empty."
    return ((get32(&uart->stat) >> 9) & 0x1) == 1;

#else
    // transmitter done: idle and empty
    return ((get32(&uart->stat) >> 9) & 0x1) == 1;

    // i think this is wrong.

    // If this bit is set the transmitter FIFO is empty. Thus it
    // FIFO is empty can accept 8 symbols.
    return ((get32(&uart->stat) >> 8) & 0x1) == 1;

#endif

}

void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        rpi_wait();
}

// nice.
void using_staff_uart(void) { }
