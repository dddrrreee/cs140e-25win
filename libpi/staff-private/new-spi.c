/* 
 * engler
 * Based on Broadcomm bcm2835 ch 10 SPI (p148-)
 * Omar did it first; I checked this against his impl, though this one
 * looks significantly different.
 *
 * XXX: better check the assumption that broadcom indicates reading = any value
 * and you have to write 0s.  I think might have mis-interpreted.  Doesn't matter
 * for our A+'s, but makes code uglier.
 * 
 * The only things you need are the spi_init() and spi_transfer() procedures
 * and these use just a handful of fields.
 *
 * Most of the structure fields are not used (e.g., for interrupts) but
 * are there if you want to tweak.   
 * In this device and others, broadcom specifies that reading unused bits
 * gives any value, but you must write back 0s
 *	---- even if you read something else ----
 * Our pattern to deal with such unspecified bits:
 *	1. always read the current value and use the appropriate clear_*() 
 * 	routine to set all unspecified bits to 0 (i.e., those bits where the 
 * 	hardware can return any value on read).
 *	2. modify this shadow copy.
 *	3. use put32/put32_T to write the shadow copy back to hardware.  These
 * 	routines are written to prevent the compiler from (1) tearing the 
 * 	value into multiple writes and (2) or storing back just a subset
 *	(such as could happen if you write a 8bit field in a 32bit struct).
 *
 * This strategy means we always write legal values as specified in Broadcom,
 * the compiler cannot mess with things, and we always write back the current
 * values that the hardware has.  The latter point is (perhaps) important:  
 * 
 * 	We do not follow the embedded pattern of setting up shadow copies of 
 * 	registers at config time and writing these back.  (We could.)  In
 * 	general if you set a field to X there is no guarantee that (1) the 
 * 	hardware does not later set it to Y where (2) reseting to X (via a 
 * 	shadow writeback) changes behavior.   Instead we assume writing the 
 * 	current value is a more robust strategy.   This also has its own 
 *	assumptions. For SPI (and I2C) it appears to be the better default.
 *
 * Notes:
 *	- some of the comments repeat.  sorry.
 * 	- Data is transmitted MSB first (p149) and (i think) reversed.
 *	- xfer = 1 byte to many.
 *	- you can use one wire (MIMO) instead of two wires (MISO and 
 *	  MOSI).  in order to read date have to (1) set the SPI_REN
 * 	  bit in the SPI control and status registers (SPI_CS) and (2)
 *	  write to the SPI_FIFO with anything and a read xaction will
 *	  take place and appear on the FIFO.
 *	- there is also a LoSSI mode (p150)
 *	- only one SPI interface; it's referred to as SPI1.  it does have
 *	  two additional mini SPI interfaces (SPI0, SPI2) in ch2.3.
 *
 * Good SPI notes:
 *  	- https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus
 * 	- https://learn.sparkfun.com/tutorials/serial-peripheral-interface-spi
 *
 * Open question:
 *	- do we need memory barriers internally?
 *
 * If speed ever matters, this can be tuned a fair amount.  easy change:
 * make *_raw versions that:
 *	- elide mem barriers and assertions.
 *	- elide bit clearing.
 *	- inline put32/get32.
 * [This could be done with preprocessor directives that compile the file
 * twice.]
 *
 * Research:
 *	- If we ever do an extensible optimizer, getting rid of duplicate mem
 * 	barriers would be a good one --- if back-to-back, cut out.  if not
 * 	different peripherals cut out.
 *
 * 	- Extensible compiler that enforces the broadcom mem model: (1)
 *	don't write indeterminate bit values, make sure to do flushes
 *	as needed, etc.
 */
#include "rpi.h"
#include "spi.h"
#include "libc/helper-macros.h"

static unsigned verbose_p = 0;
void spi_set_verbose(unsigned v_p) {
    verbose_p = v_p;
}

enum { spi_checking_p = 0 };

#define spi_debug(args...) \
    do { if(verbose_p) debug(args); } while(0)

// #define spi_barrier() dmb()
#define spi_barrier() dev_barrier()


// I don't believe we can use bitfields directly.
// 	1.  in some cases might have to set two fields at the
//	    same time.
//      2. in many cases the unused bits *must* be written as 0, however,
//	   their value on read is indetermininate.   unfortunately, when you
// 	   assign a non aligned / non 8,16,32 bitfield, the compiler will
//	   do a read-modify-write, which will read this indeterminate
//	   value and then write it back.  if it's non-zero (legal), then
// 	   this write will be illegal.
//
//	so we instead do all operations off to the side, then retype it
//	as an unsigned and assign to the main one.
//
// 	this gives us the advantage of not having to do or/and and clear,
// 	define a lot of constants.
struct cs { // off = 0x0
	unsigned 	
		cs:2, 	// 0:1  
			// chip select: 00 = select 0,  p155
			// 01 = 1, ...
	       	cpha:1,	// 2   clock phase

		cpol:1,	// 3   clock polarity
			// 0 = rest state of clock = low
			// 1 = rest state of clock = hi

		// if TA and CLEAR are set at the same
		// time the FIFOs are cleared before the new frame 
		// is started.  p155
		clear:2, // 4:5 clear fifo
			// 0b00 = no action
			// 0bx1 = clear TX fifo
			// 0b1x = clear RX fifo on shot.
		
		cspol:1, // chip select polarity.
			// 0 = chip select line = active low
			// 1 = chip select line = active high

		ta:1,   // transfer actie.
			// 0 = transfer not active/CS lines are all 
			// high (assuming cspol=0).   RXR and DONE = 0.
			// 1 = transfer active, writes to SPIFIFO
			// write data to TX FIFO.   TA is cleared by
			// a dma frame and pulse from dma controller(?)
		dmaen:1, // :8 dma enable.  
			// 0 = no dma
			// 1 = enable dma.
		intd:1, // :9 interrupt done
			// 0 = dont' generate interrupt on xfer complete.
			// 1 = generate int when DONE=1 (how to check it was
			// this int?)
		intr:1, // :10 interrupt on rx
			// 0 = no int.
			// 1 = int while RXR = 1.
		adcs:1, // :11 auto de-assert chip select.
			// 0 = don't auto deslect at end of DMA
			// 1 = auto de-assert chip at end of DMA
		ren:1,	// :12 read enable if using bi-dir mode.  
			// 0 = we will write to SPI
			// 1 = we will read from SPI.
		len:1,  // :13 LEN LoSSI enable.  configure as lossi master.
			// 0 = SPI master
			// 1 = LoSSI master.
		__unused_lmono:1, // :14 unused.
		__unused_te_en:1, // :15 unused
		done:1,  // :16  Done transfer.
			// 0 = transfer in progress (or not active TA = 0)
			// 1 = transfer complete.   cleared by writing
			// more to tx fifo, or setting TA = 0.
		rxd:1,	// :17 RX FIFO contains data.
			// 0 = empty
			// 1 = at least one byte.
		txd:1,	// :18 tx can accept data
			// 0 = tx fifo is full so wait.
			// 1 = tx fifo has space for at least 1 byte.
		rxr:1,  // :19 rx fifo needs reading
			// 0 = rx fifo is less than full (or not active TA=0)
			// 1 = rx fifo is "or more full"?  cleared by reading 
			// sufficient data or setting TA to 0. 
		rxf:1,  // :20 rx full
			// 0 = rx not full
			// 1 = rx fifo is full.  no further data will
			// be sent / received until data read from fifo.
		cspol0:1,  // :21 chip0 polarity
			// 0 = active low, 1 = active high.
		cspol1:1, // :22
		cspol2:1, // :23
		dma_len:1, // :24 enable dma in lossi mode.
		len_long:1, // 25 enable long data word in lossi mode if
			// dma_len = 1.  
			// 0 = write to FIFO will write a byte
			// 1 = write will write 32 bit word.
		__unused0:6
	;
	
};	


static struct cs new_cs(void) {
    return (struct cs) {};
}

static struct cs read_cs(volatile struct cs *c) {
        return get32_T(c);
}

typedef volatile struct {
	volatile struct cs CS;

	// p156: allows TX data to be written to TX fifo and RX to be read 
	// from RX fifo.
	//
	// if TA is clear and DMAEN is set, the first 32-bit write to FIFO 
	// will control SPIDLEN and SPICS.   Subsequent reads and writes 
	// will be taken as 4-byte values to be read/written to the FIFOs.
	//
	// if TA is set and DMAEN is clear, write to the register write bytes
	// to the TX (4-byte??), reads from it read from the RX FIFO
	volatile unsigned FIFO;  // off = 0x4

	//p156:  allows clock rate to be set.  clock divisor, must be 
	// power of 2.   0 = divisor is 65536 (XXX 1 = 32k?).  upper 
	// bits 31:16 = 0 write, don't read.
	volatile unsigned CLK;	// off = 0x8
	// only valid for DMA: set length of transfer.   31:16 = don't use, 0.
	volatile unsigned DLEN;  // off = 0xc
	// losi hold delay to be set.   0:3 bits.  reset = 1.
	volatile unsigned LTOH;
	// p157 this is for DMA panics.   weird reset values.  i don't think
	// we need to use it.  XXX
	volatile unsigned DC;
} SPI;

// p 152
// should have 0,1,2? --- is this 0 or 1?
static SPI *spi = (void*)0x20204000;
enum { clear_tx_rx = 0b11 };

/*
	p158 10.6.1 Polled
	a) Set CS, CPOL, CPHA as required and set TA = 1.
	b) Poll TXD writing bytes to SPI_FIFO, RXD reading bytes from
	   SPI_FIFO until all data written.
	c) Poll DONE until it goes to 1.
	d) Set TA = 0.
 */
static void start_transfer(spi_t s, unsigned nbytes) {
	spi_barrier();		// paranoid, but would suck to debug if needed.

    spi_debug("about to tx chip=%d\n", s.chip);
	struct cs c = read_cs(&spi->CS);

	// broadcom: set these at the same time for the next transfer.
	c.clear = clear_tx_rx;
	c.ta = 1;
    // i think have to specify the chip each time.
    c.cs = s.chip;
	put32_T(spi->CS, c);

    // this is just paranoid
    if(spi_checking_p) {
	    c = read_cs(&spi->CS);
        demand(c.cs == s.chip, "chip=%d, cs=%d\n", s.chip, c.cs);
    }

	/*
 	 * "cs line will be asserted at least 3 clock cycles before the msb 
  	 * of the first byte of the transfer" 
	 * 
	 * i believe that means we need a 3 clock cycle delay after writing
	 * cs.  given we are running uncached, volatile, etc i think this 
	 * will always be guaranteed.  however, we insert a delay just in case.
	 * do we have to flush write buffers?
	 */
	spi_barrier();		// if we ran cached i think we'd have to 
				// force out through the write buffer right?
	delay_cycles(3);
}

static void end_transfer(spi_t s) {
	// wait til done so we can reset ta.
    
    if(spi_checking_p)
	    assert(spi->CS.cs == s.chip);

    spi_debug("about to wait til done chip=%d\n", s.chip);
	while(!spi->CS.done)
		;
    spi_debug("done chip=%d\n", s.chip);

	// I thought done=1 implies ta=0, but it appears we have to reset it.
	struct cs c = read_cs(&spi->CS);
	c.ta = 0; 	
    if(c.cs != s.chip)
        panic("selected wrong chip: cs=%d, chip=%d\n", c.cs, s.chip);

	put32_T(spi->CS, c);
	spi_barrier();		// paranoid, but would suck to debug if needed
	delay_cycles(3); 	// probably overkill; needed?
}

/*
 *
 * confusing: we have to communicate bi-directionally, even if 
 * sending one way.
 * p.159: 
 *	The SPI Master knows nothing of the peripherals it is connected
 *	to. It always both sends and receives bytes for every byte of
 *	the transaction.
 *
 * and this is true in general:
 *
 *  https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus
 *	During each SPI clock cycle, a full duplex data transmission
 *	occurs. The master sends a bit on the MOSI line and the slave
 *	reads it, while the slave sends a bit on the MISO line and the
 *	master reads it. This sequence is maintained even when only
 *	one-directional data transfer is intended.
 * 
 * i think we can actually read at the end if the fifo is big enough,
 * which most likely it will be.  however, the gain in speed (by 
 * eliminating polling) probably just doesn't matter.
 */
static uint8_t shift_byte(unsigned char b) {
	// while tx is full: i think this cannot happen given how we interact
	// with the device.
	while(!spi->CS.txd)
		;
	put32(&spi->FIFO, (unsigned) b);

	// rx is empty
	while(!spi->CS.rxd)
		;
    // wait: what: if you change this to get32_raw, doesn't work: not aligned?
	return get32(&spi->FIFO)&0xff;
}

#define MOVE4

#if defined(MOVE1)
// inlining get32/put32 doesn't seem to matter.
static void move_data(uint8_t *rx, const uint8_t *tx,  unsigned nbytes) {
    
    unsigned recv = 0, sent = 0;
    for(; recv < nbytes; ) {
        // we preferentially send as much as we can as soon as we can.
        if(sent < nbytes && spi->CS.txd)
	        spi->FIFO =  tx[sent++];

        // pull off one at a time.
	    if(spi->CS.rxd)
	        rx[recv++] = spi->FIFO & 0xff;
    }
    if(sent != nbytes)
        panic("sent=%d, recv=%d, nbytes=%d\n", sent, recv, nbytes);
    assert(sent == nbytes);
    assert(recv == nbytes);
}

#elif defined(MOVE2)

static void move_data(uint8_t *rx, const uint8_t *tx,  unsigned nbytes) {
    
    unsigned recv = 0, sent = 0;
    for(; recv < nbytes; ) {
        // we preferentially send as much as we can as soon as we can.
        if(sent < nbytes && spi->CS.txd)
	        put8(&spi->FIFO, tx[sent++]);

        // pull off one at a time.
	    if(spi->CS.rxd)
	        rx[recv++] = get8(&spi->FIFO) & 0xff;
    }
    if(sent != nbytes)
        panic("sent=%d, recv=%d, nbytes=%d\n", sent, recv, nbytes);
    assert(sent == nbytes);
}

#elif defined(MOVE3)

static void move_data(uint8_t *rx, const uint8_t *tx,  unsigned nbytes) {
    unsigned recv = 0, sent = 0;
    for(; recv < nbytes; ) {
        // we preferentially send as much as we can as soon as we can.
        for(; sent < nbytes && spi->CS.txd; sent++)
	        put8(&spi->FIFO, tx[sent]);

        // pull off one at a time.
	    if(spi->CS.rxd)
	        rx[recv++] = get8(&spi->FIFO) & 0xff;
    }
    if(sent != nbytes)
        panic("sent=%d, recv=%d, nbytes=%d\n", sent, recv, nbytes);
    assert(sent == nbytes);
    assert(recv == nbytes);
}

#elif defined(MOVE4)

static void move_data(uint8_t *rx, const uint8_t *tx,  unsigned nbytes) {
    unsigned initial = nbytes < 16 ? nbytes : 16;

    // blindly send the first 16 [huh: seems to not matter]
    unsigned sent = 0;
    for(; sent < initial; sent++)
        put32(&spi->FIFO, tx[sent]);

    unsigned recv = 0; 
    for(; recv < nbytes; ) {
        // we preferentially send as much as we can as soon as we can.
        for(; sent < nbytes && spi->CS.txd; sent++)
	        put32(&spi->FIFO, tx[sent]);

        // pull off one at a time.
	    if(spi->CS.rxd)
	        rx[recv++] = get32(&spi->FIFO) & 0xff;
    }
    if(sent != nbytes)
        panic("sent=%d, recv=%d, nbytes=%d\n", sent, recv, nbytes);
    assert(sent == nbytes);
    assert(recv == nbytes);
}

#elif defined(MOVE5)

static void move_data(uint8_t *rx, const uint8_t *tx,  unsigned nbytes) {
    unsigned initial = nbytes < 16 ? nbytes : 16;

    // blindly send the first 16 [huh: seems to not matter]
    unsigned sent = 0;
    for(; sent < initial; sent++)
        spi->FIFO = tx[sent];

    unsigned recv = 0; 
    for(; recv < nbytes; ) {
        // we preferentially send as much as we can as soon as we can.
        for(; sent < nbytes && spi->CS.txd; sent++)
	        spi->FIFO = tx[sent];

        // pull off one at a time.
	    if(spi->CS.rxd)
	        rx[recv++] = spi->FIFO & 0xff;
    }
    if(sent != nbytes)
        panic("sent=%d, recv=%d, nbytes=%d\n", sent, recv, nbytes);
    assert(sent == nbytes);
    assert(recv == nbytes);
}
#endif


// < 0 = error; none yet
// XXX: can we push a bunch at once?
int spi_n_transfer(spi_t s, uint8_t rx[], const uint8_t tx[], unsigned nbytes) {

    assert(nbytes);
	start_transfer(s, nbytes);
        if(nbytes)
            move_data(rx,tx,nbytes);
        else {
		    for(int i = 0; i < nbytes; i++)
			    rx[i] = shift_byte(tx[i]);
        }
	end_transfer(s);

	// XXX: need to go through doc more for the errors that can occur.
	return 1;
}

// check bitfield positions.
#define check_bit_pos(_struct, field, n) do {                           \
        union _u {                                                      \
                _struct s;                                              \
                unsigned u;                                             \
        } x = (union _u) { .s = (_struct){ .field = 1 } };              \
        unsigned exp = 1<<n;                                            \
        if(x.u != exp)                                                  \
                panic("expected %x got %x for %s\n", exp,x.u,#field);   \
        assert(x.u == exp);                                             \
} while(0)


static void check_layout(void) { 
	// static layout checks.
	AssertNow(offsetof(SPI, CS) == 0x0);
	AssertNow(offsetof(SPI, FIFO) == 0x4);
	AssertNow(offsetof(SPI, CLK) == 0x8);
	AssertNow(offsetof(SPI, DLEN) == 0xc);
	AssertNow(offsetof(SPI, LTOH) == 0x10);
	AssertNow(offsetof(SPI, DC) == 0x14);
	AssertNow(sizeof (struct cs) == 4);

	check_bit_pos(struct cs, cs,	0);
	check_bit_pos(struct cs, cpha,	2);
	check_bit_pos(struct cs, cpol,	3);
	check_bit_pos(struct cs, clear,	4);
	check_bit_pos(struct cs, cspol,	6);
	check_bit_pos(struct cs, ta,	7);
	check_bit_pos(struct cs, dmaen,	8);
	check_bit_pos(struct cs, intd,	9);
	check_bit_pos(struct cs, intr,	10);
	check_bit_pos(struct cs, adcs,	11);
	check_bit_pos(struct cs, ren,	12);
	check_bit_pos(struct cs, len,	13);
	check_bit_pos(struct cs, __unused_lmono,	14);
	check_bit_pos(struct cs, __unused_te_en,	15);
	check_bit_pos(struct cs, done,	16);
	check_bit_pos(struct cs, rxd,	17);
	check_bit_pos(struct cs, txd,	18);
	check_bit_pos(struct cs, rxr,	19);
	check_bit_pos(struct cs, rxf,	20);
	check_bit_pos(struct cs, cspol0,	21);
	check_bit_pos(struct cs, cspol1,	22);
	check_bit_pos(struct cs, cspol2,	23);
	check_bit_pos(struct cs, dma_len,	24);
	check_bit_pos(struct cs, len_long,	25);
}

enum {
    SPI0_CE0_N = 8,
    SPI0_CE1_N = 7,
    SPI0_MISO = 9,
    SPI0_MOSI = 10,
    SPI0_SCLK = 11
};


// hardwired now to setup for spi0.   
spi_t spi_n_init(unsigned chip_select, unsigned clk_div) {
	check_layout();
    
    static unsigned clk_div_val;
    if(!clk_div_val)
        clk_div_val = clk_div;
    else if(clk_div != clk_div_val)
        panic("all spi calls must have the same clock freq: old=%d, new=%d\n", 
            clk_div_val, clk_div);

	unsigned chip;
	dev_barrier();

	// SPI0_CE0_N
	if(chip_select == 0) {
		gpio_set_function(SPI0_CE0_N, GPIO_FUNC_ALT0); 
		chip = 0b00;
	// SPI0_CE1_N
	} else if(chip_select == 1) {
		gpio_set_function(SPI0_CE1_N, GPIO_FUNC_ALT0); 
		chip = 0b01;
	} else
		panic("invalid chip select=%x\n", chip_select);

    // only do this a single time, ever.
    static int init_p;
    if(!init_p) {
        init_p = 1;
		gpio_set_function(SPI0_MISO, GPIO_FUNC_ALT0); // SPI0_MISO
		gpio_set_function(SPI0_MOSI, GPIO_FUNC_ALT0); // SPI0_MOSI
		gpio_set_function(SPI0_SCLK, GPIO_FUNC_ALT0); // SPI0_SCLK
    }

	dev_barrier();

	// construct the current copy.
	struct cs c = new_cs();
	c.cs = chip;
	c.clear = clear_tx_rx;		// reset to clean state.
	put32_T(spi->CS, c);

    dev_barrier();

    c = read_cs(&spi->CS);
    demand(c.cs == chip, "chip=%d, cs=%d\n", chip, c.cs);

	// power of 2.
	//assert((clk_div & -clk_div) == clk_div);
	//assert(clk_div >> 15 == 0);
	put32(&spi->CLK, clk_div);

	dev_barrier();

    return (spi_t) { .chip = chip };
}
