/*
 * engler
 * simplified i2c implementation --- no dma, no interrupts.  the latter
 * probably should get added.  the pi's we use can only access i2c1
 * so we hardwire everything in.
 *
 * this implementation is alot longer than cs107e's i2c:
 *	1. partly this is b/c we define the full structure of all the 
 * 	different i2c flags, but don't use most of them.    plus lots of
 *	comments and a bunch of code for sanity checking that is missing
 *	from the cs107e code.
 *
 *	2. the main complexity comes from the fact that many broadcom
 *	chip registers contain unused bits where the rules are (1) you must
 *	write 0s to these bits and (2) reads of these bits can produce 
 * 	any value.
 *
 *	cs107e makes the strong (but perhaps correct) assumption that
 *	reading these unused bits gives 0s.   as a result given a
 *	hardware address <addr> that contains unused bits it can do
 *		*addr |= <some value>
 *	to bitwise or in a new value.   we don't want to assume 
 *	reads return 0, so we initially constructed a default shadow copy 
 *	of the current config (with unused bits set to 0) and when we want to 
 *	mutate values, apply them to a copy of this shadow and write 
 *	the shadow out when done.
 *	
 *	improvement: actually, the shadow has the assumption that when
 *	you read later, the hw still has these initial values in the 
 *	fields.  so we switched to a more direct way of read, set all 
 *	undef to 0, modify, and then write out.
 *
 *	3. also, cs107e code makes the strong assumption (but again
 *	maybe correct) that 
 *		*addr = val 
 * 	or 
 *		struct->field |= val
 *	is *guaranteed* to result in a single 32-bit store/load rather than 
 * 	potentially having a single 8 bit (or 16 bit) load / store
 *	(or even multiple stores).  this might be a reasonable assumption.
 *	however, it's actually a big ask to guarantee that gcc under any
 *	optimization level you give it (esp no -O) with lots of weird
 *	addresses and volatile will always do single 32-bit stores even
 *	when you are modifying sub 32-bit quantities in the word.  
 *
 *	related: if you do structure assignment:
 *		foo(struct foo *dst, struct foo *src) {
 *			*dst = *src;
 *		}
 *	gcc may call memcpy and the current implementation is a byte copy.
 *	broadcom says nothing (afaik) that reassures that writing a byte
 *	at a time to a 32 bit broadcom register is gonna necessarily be ok.
 * 
 * Starts at p28.
 *
 * There are three BSC masters inside BCM. The register addresses starts from
 *	 BSC0: 0x7E20_5000
 *	 BSC1: 0x7E80_4000
 *	 BSC2 : 0x7E80_5000
 * the PI only has BSC1.
 *
 *
 * Notes:
 * 	- p36 has how to use 10 bit i2c addresses.
 *
 *	- is doing a load byte from a 32 bit hardware register *guaranteed*
 *	  to work as expected?  or do we have to force a get32?
 *
 * 	- make it so you can plug in new memory models into the compiler.
 *	  the pi model is a weird one, easy to mess up.  (e.g., reserved
 *	  bits = 0 on write, = * on read, must mb() across peripheral
 *	  reads).
 *
 *	- need to prevent all uses of raw load/store.  if you go through and
 *	rewrite the whole thing w/ get/put then you should be able to 
 *	run on laptop and just use remote execution.   should try this.  yes :)
 * 
 *  - can view all traces of loads and stores and warn if switch devices 
 *  without a mem-barrier
 */
#include "rpi.h"
#include "libc/helper-macros.h"
#include "i2c.h"

/* 
 * p30 we explicitly 0 out all unamed locations that can return indeterminate 
 * values but must have a write = 0.
 */
typedef struct {
	uint32_t read:1,	// :0 READ transfer.  
				// 0 = write packet, 1 = read packet.
		__unused0:3,		// 1:3, reserved: write as 0.
		clear:2,	// 4:5 fifo clear
				// 0b00 = no action
				// 0bx1 = clear fifo, one shot
				// 0b1x = clear fifo, one shot
				// read back as 0.
				// 2-bits b/c back-compat.
		__unused1:1,		// :6 write as 0
		st:1,		// :7, start xfer
				// 0 = no action
				// 1 = start a new xfer.  
				// one shot, read as 0.
		intd:1,		// :8, intd interrupt done
				// 0 = don't generate interrupts on done.
				// 1 = generate interrupt DONE = 1.
		intt:1,		// :9 interrupt on tx
				// 0 = don't generate ints on txw condition
				// 1 = generate interupt while TXW = 1
		intr:1,		// :10 interrupt on rx.
				// 0 = don't interrupt
				// 1 = generate interrupt while RXR = 1.
		__unused2:4,		// 11:14 - reserved, write 0.
		i2cen:1,	// 15 i2c enable
				// 0 = disabled
				// 1 = enabled.
		__unused3:16;		// reserved: write as 0.
} c_register;
static c_register clear_c(c_register c) {
	c.__unused0 = 0;
	c.__unused1 = 0;
	c.__unused2 = 0;
	c.__unused3 = 0;
	return c;
}
static c_register read_c(volatile c_register *c) {
	return clear_c(get32_T(c));
}

// p31 status register: recover status, errors, int requests.
// thorough description.
typedef struct {
	unsigned ta:1,	// :0
				// 0 = transfer not active,
				// 1 = xfer active.
		done:1, 	// :1
				// 0 = xfer not completed
				// 1 = xfer complete.  
				// *cleared by writing 1 to field.
		txw:1,		// :2 fifo needs writing (full)
				// 0 = FIFO is at least full and a write 
				// is underway.
				// 1 = FIFO is less than full and a write is
				// underway.
				// cleared by writing enough data to FIFO(?)
		rxr:1,		// :3 fifo needs reading.
				// 0 = FIFO < full and read is underway(?)
				// 1 = FIFO is more full and a read is 
				// underway(?)
				// cleared by reading sufficient data from 
				// FIFO
		txd:1,		// :4 fifo can accept data.
				// 0 = fifo is full.
				// 1 = fifo has space for at least 1 byte.
		rxd:1,		// :5 fifo contains data
				// 0 = fifo is empty
				// 1 = fifo has at least 1 byte.
				// cleared by reading.
		txe:1,		// :6 fifo empty
				// 0 = fifo not empty
				// 1 = fifo is empty.
				// if write underway no further serial 
				// data can be xmitted(?)
		rxf:1,		// :7 fifo full
				// 0 = fifo not full
				// 1 = fifo full.  must read
		err:1,		// :8 acck error
				// 0 = no errors
				// 1 = slave not ack address
				// *cleared by writing 1.
		clktk:1,	// :9 clock stretch timeout
				// 0 = no errors detect
				// 1 = slave held scl signal low
				// for longer than specified in icclktk
				// * cleared by writing 1.
		__unused0:22;
} s_register;


static s_register clear_s(s_register s) {
	s.__unused0 = 0;
	return s;
}
static s_register read_s(volatile s_register *s) {
	return clear_s(get32_T(s));
}

typedef struct RPI_i2c {
	c_register control;
	s_register status;

#	define check_dlen(x) assert(((x) >> 16) == 0)
	uint32_t dlen; 			// number of bytes to xmit, recv
					// reading from dlen when TA=1
					// or DONE=1 returns bytes still
					// to recv/xmit.  
					// reading when TA=0 and DONE=0
					// returns the last DLEN written.
					// can be left over multiple pkts.

#	define check_dev_addr(x) assert(((x) >> 7) == 0)
	uint32_t 	dev_addr;
	// only use the lower 8 bits.
	uint32_t fifo;
#	define check_clock_div(x) assert(((x) >> 16) == 0)
	uint32_t clock_div;
	// we aren't going to use this.
	uint32_t clock_delay;
	// we aren't going to use this.
	uint32_t clock_stretch_timeout;
} RPI_i2c;

// 0x20205000 	// BSC0
// 0x20805000 	// BSC2
static volatile RPI_i2c *i2c = (void*)0x20804000; 	// BSC1

#define wait_for(_cond) do { 					\
	while(!(_cond)) {					\
		if(i2c->status.err)				\
			panic("i2c slave ack error");		\
		if(i2c->status.clktk)				\
			panic("i2c slave held line too long"); 	\
	}							\
} while(0)

#if 0
		/* can't be done yet! */	
		assert(i2c->status.ta == 1);
#endif

static void reset_status(void) { 
	s_register s = read_s(&i2c->status);
		s.done = 1; 	// *cleared by writing 1 to field.
		// clear previous errors (if any)
		// currently we should never have errors (we assert fail),
		// but have the code for future.
		s.err = 1;	// cleared by writing 1
		s.clktk = 1;	// cleared by writing 1
	put32_T(i2c->status, s);
}

static void xfer_start(unsigned addr, unsigned nbytes, int read_p) {
    dev_barrier(); 				// paranoid.

	// T1=1 --> in middle of xfer.
	wait_for(i2c->status.ta == 0);

	// 1 = fifo is empty.
	assert(i2c->status.txe == 1);

	reset_status();

	// set slave address 
	check_dev_addr(addr);
	put32(&i2c->dev_addr, addr);

	// write length before initiating xfer.
	check_dlen(nbytes);
	put32(&i2c->dlen, nbytes);

	c_register c = read_c(&i2c->control);
		c.clear = 0b11; // shouldn't need, but hygenic(?)
		c.st = 1;
		c.read = read_p;
	put32_T(i2c->control, c);

	// do we have to force the write out past the pi's dcache
	// writebuffers?
	// i would say so in general, but code does not seem to worry
	// about this.   periph addresses = uncached?

	// T1=0 --> our transfer has not started, so wait.
	wait_for(i2c->status.ta == 1);
}

static void xfer_end(void) {
	// DONE=1 --> xfer completes.
	wait_for(i2c->status.done == 1);

	// i think done implies xfer in progress is false, right?
	assert(i2c->status.ta == 0);
	// if we get this far, can't have error, right?
	assert(!i2c->status.err);

	dev_barrier(); // paranoid; do full bi-directional barrier.
}

// this should be able to fail, right??
int i2c_write(unsigned addr, uint8_t data[], unsigned nbytes) {
	xfer_start(addr, nbytes, 0);

	for(unsigned i = 0; i < nbytes; i++) {
		// TXD = 1 --> more room for more bytes.
		wait_for(i2c->status.txd == 1);

		// XXX: need to change this to get/put
		i2c->fifo = data[i];
	}

	xfer_end();
	return 1;
}

// should be able to fail, right?
int i2c_read(unsigned addr, uint8_t data[], unsigned nbytes) {
	xfer_start(addr, nbytes, 1);

    // wait: what about block read?
	for(unsigned i = 0; i < nbytes; i++) {
		// RXD = 1 --> at least 1 byte of data available.
		wait_for(i2c->status.rxd == 1);

		// XXX: need to change this to get/put
		data[i] = i2c->fifo&0xff;
	}

	xfer_end();
	return 1;
}

static void check_layout(void); 
static int initialized_p = 0;

int i2c_is_initialized(void) {
	return initialized_p;
}
void i2c_init_once(void) {
	if(i2c_is_initialized())
		return;
	i2c_init();
}

// could bind addr here, or could redo?
// is the i2c dev different than gpio?  yes?  but
// can only talk to it via gpio so maybe we don't
// need dev barriers.
void i2c_init(void) {
	check_layout();

    // we don't know what was happening before.
	dev_barrier();

        // these are supposed to be pullups --- check
        // w/ multimeter.
		const unsigned SDA = 2; // GPIO_PIN2;
		const unsigned SCL = 3; // GPIO_PIN3;
		gpio_set_function(SDA, GPIO_FUNC_ALT0);
		gpio_set_function(SCL, GPIO_FUNC_ALT0);

    // we are using a different device, better dev_barrier
	dev_barrier();

		// set initial defaults.
		// 	XXX: we ignore clock div and del.
		c_register c = u32_to_T(c_register, 0);
		c.i2cen = 1; 	// enabled.
		put32_T(i2c->control,c);

    // i think this is redundant, right?
	dev_barrier();

		reset_status();

    // don't know what device is going to get used next.
	dev_barrier();
	assert(i2c->status.ta == 0);

	initialized_p = 1;
}

// shortest will be 130 for i2c accel.
void i2c_init_clk_div(unsigned clk_div) {
	check_layout();

    // we don't know what was happening before.
	dev_barrier();

        // these are supposed to be pullups --- check
        // w/ multimeter.
		const unsigned SDA = 2; // GPIO_PIN2;
		const unsigned SCL = 3; // GPIO_PIN3;
		gpio_set_function(SDA, GPIO_FUNC_ALT0);
		gpio_set_function(SCL, GPIO_FUNC_ALT0);

    // we are using a different device, better dev_barrier
	dev_barrier();

		// set initial defaults.
		// 	XXX: we ignore clock div and del.
		c_register c = u32_to_T(c_register, 0);
		c.i2cen = 1; 	// enabled.
		put32_T(i2c->control,c);

        i2c->clock_div = clk_div;



    // i think this is redundant, right?
	dev_barrier();

		reset_status();

    // don't know what device is going to get used next.
	dev_barrier();
	assert(i2c->status.ta == 0);

	initialized_p = 1;
}



// sanity check that sizes and offsets are correct.
// would be nice if the bitchecks could be compile time.
static void check_layout(void) { 
    AssertNow(offsetof(RPI_i2c, control) == 0x0);
    AssertNow(offsetof(RPI_i2c, status) == 0x4);
    AssertNow(offsetof(RPI_i2c, dlen) == 0x8);
    AssertNow(offsetof(RPI_i2c, dev_addr) == 0xc);
    AssertNow(offsetof(RPI_i2c, fifo) == 0x10);
    AssertNow(offsetof(RPI_i2c, clock_div) == 0x14);
    AssertNow(offsetof(RPI_i2c, clock_delay) == 0x18);
    AssertNow(offsetof(RPI_i2c, clock_stretch_timeout) == 0x1c);

    AssertNow(sizeof(c_register) == 4);
	check_bitfield(c_register, read, 0, 1);
	check_bitfield(c_register, clear, 4, 2);
	check_bitfield(c_register, st, 7, 1);
	check_bitfield(c_register, intd, 8, 1);
	check_bitfield(c_register, intt, 9, 1);
	check_bitfield(c_register, intr, 10, 1);
	check_bitfield(c_register, i2cen, 15, 1);

    AssertNow(sizeof(s_register) == 4);
	check_bitfield(s_register, ta, 0, 1);
	check_bitfield(s_register, done, 1, 1);
	check_bitfield(s_register, txw, 2, 1);
	check_bitfield(s_register, rxr, 3, 1);
	check_bitfield(s_register, txd, 4, 1);
	check_bitfield(s_register, rxd, 5, 1);
	check_bitfield(s_register, txe, 6, 1);
	check_bitfield(s_register, rxf, 7, 1);
	check_bitfield(s_register, err, 8, 1);
	check_bitfield(s_register, clktk, 9, 1);
}
