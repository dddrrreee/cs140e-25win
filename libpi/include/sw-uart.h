#ifndef __SW_UART_H__
#define __SW_UART_H__

// engler, cs140e: a simple software uart interface that bit-bangs
// the uart protocol on two client-specified GPIO pins at a 
// client-specified baud rate.
//
// you'll have to finish writing:
//  - <sw_uart_put8>
//  - <sw_uart_get8> 
//  - <sw_uart_init_helper>
// in sw-uart.c


// simple structure for sw-uart.  you'll likely have to extend 
// in the future.
//  - rx: the GPIO pin used for receiving.  (make sure you 
//    set as a GPIO input!)
//  - tx: the GPIO pin used for transmitting.  (make sure 
//    you set as GPIO output!)
//  - the baud rate: initially 115200, identical to what we 
//    have been using for the UART all quarter.  if you change this, 
//    you will also have to change the baud rate in your unix side.
//    (e.g., in pi-cat)!
//  - cycle_per_bit: the number of cycles we have to hold 
//    the TX low (0) or high (1) to transmit a bit.  or, conversely, 
//    the number of cycles the RX pin will be held low or high 
//    by the sender.
//  - usec_per_bit: the number of microseconds needed to transmit a bit.
//    note: this is pretty coarse, so won't work at higher baud rates.
typedef struct {
    uint8_t tx,             // gpio output pin: default on.
            rx;             // gpio input pin
    uint32_t baud;          // 115200 default.
    uint32_t cycle_per_bit; // cycles b/n each bit = (baud / cpu MHz)
    uint32_t usec_per_bit;  // microseconds b/n each bit = (1M/cpu MHz).
} sw_uart_t;

// NOTE: you must implement the next three routines.

// writes out 8-bits <b> to the sw-uart <s>.  
void sw_uart_put8(sw_uart_t *s, uint8_t b);

// read 8 bits from input sw-uart <s>.
//  - returns -1 if no input after <timeout_usec>
//  - if timeout_usec = 0, will returns -1 right away if 
//    there is no data.
int sw_uart_get8_timeout(sw_uart_t *s, uint32_t timeout_usec);

// create the <sw_uart_t> structure, setup the GPIO pins.
sw_uart_t sw_uart_init_helper(
        unsigned tx, 
        unsigned rx, 
        unsigned baud, 
        unsigned cyc_per_bit,
        unsigned usec_per_bit);


// we pre-compute cycles per bit b/c the arm1176 does not 
// have a divide instruction.  computing at compile time
// (using the below macros) lets gcc see the constants used
// and thus replace them with cheaper operations (that the
// hardware does provide).
// we need to dthis 
//
// NOTE: if you overclock the pi (or somehow you have a batch
// that doesn't run at 700mz by default) you'd have to change 
// the 700 in <BAUD_TO_CYCLES> 
#define BAUD_TO_CYCLES(baud) ((700*1000*1000UL)/baud)
#define BAUD_TO_USEC(baud) ((1000*1000UL)/baud)

// do division at the callsite so gcc can strength reduce.
// calls out to <sw_uart_init_helper> to setup GPIO state.
#define sw_uart_init(tx,rx,baud)                \
 sw_uart_init_helper(tx,rx, baud,               \
    BAUD_TO_CYCLES(baud), BAUD_TO_USEC(baud))

// create config equal to the default HW setup.
// pin 14 for TX
// pin 15 for RX
// baud = 115200
static inline sw_uart_t 
sw_uart_default(void) {
    return sw_uart_init(14,15,115200);
}


/**********************************************************
 * staff implementations.
 */

// blocking get8.  it does blow up after a large number
// of seconds.
static inline int sw_uart_get8(sw_uart_t *s) {
    // if timeout w/in 5 seconds there must be an issue.
    unsigned sec = 5;

    int c = sw_uart_get8_timeout(s, 1000*1000*sec);
    if(c<0)
        panic("timed out in %dsec: something is wrong\n",sec);
    return c;
}

// output a null-terminated string.
static inline void 
sw_uart_putk(sw_uart_t *s, const char *msg) {
    assert(msg);

    for(; *msg; msg++)
        sw_uart_put8(s, *msg);
}

// printk that uses sw_uart [should return nbytes printed?]
static inline void 
sw_uart_printk(sw_uart_t *s, const char *fmt, ...) {
    char buf[1024];
   va_list args;

    va_start(args, fmt);
       vsnprintk(buf, sizeof buf, fmt, args);
    va_end(args);

    sw_uart_putk(s, buf);
}


#endif
