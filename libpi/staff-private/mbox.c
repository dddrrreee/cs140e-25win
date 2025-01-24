//
// rpi mailbox interface.
//  https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
//
//  a more reasonable but unofficial writeup:
//  http://magicsmoke.co.za/?p=284
//
#include "rpi.h"

enum { OneMB = 1024 * 1024 };

/***********************************************************************
 * mailbox interface
 */

#define MAILBOX_FULL   (1<<31)
#define MAILBOX_EMPTY  (1<<30)
#define MAILBOX_START  0x2000B880
#define GPU_MEM_OFFSET    0x40000000

// document states: only using 8 right now.
#define MBOX_CH  8

/*
    REGISTER	ADDRESS
    Read	        0x2000B880
    Poll	        0x2000B890
    Sender	        0x2000B894
    Status	        0x2000B898
    Configuration	0x2000B89C
    Write	        0x2000B8A0
 */
#define MBOX_READ   0x2000B880
#define MBOX_STATUS 0x2000B898
#define MBOX_WRITE  0x2000B8A0

// need to pass in the pointer as a GPU address?
static uint32_t uncached(volatile void *cp) { 
    // not sure this is needed.
	return (unsigned)cp | GPU_MEM_OFFSET; 	
}

void mbox_write(unsigned channel, volatile void *data) {
    assert(channel == MBOX_CH);
    // check that is 16-byte aligned
	assert((unsigned)data%16 == 0);

    // 1. we don't know what else we were doing before: sync up 
    //    memory.
    dev_barrier();

    // 2. if mbox status is full, wait.
	while(GET32(MBOX_STATUS) & MAILBOX_FULL)
        ;

    // 3. write out the data along with the channel
	PUT32(MBOX_WRITE, uncached(data) | channel);

    // 4. make sure everything is flushed.
    dev_barrier();
}

unsigned mbox_read(unsigned channel) {
    assert(channel == MBOX_CH);

    // 1. probably don't need this since we call after mbox_write.
    dev_barrier();

    // 2. while mailbox is empty, wait.
	while(GET32(MBOX_STATUS) & MAILBOX_EMPTY)
            ;

    // 3. read from mailbox and check that the channel is set.
	unsigned v = GET32(MBOX_READ);

    // 4. verify that the reply is for <channel>
    if((v & 0xf) != channel)
        panic("impossible(?): mailbox read for a different channel\n");

    // return it.
    return v;
}

unsigned mbox_send(unsigned channel, volatile void *data) {
    mbox_write(MBOX_CH, data);
    mbox_read(MBOX_CH);
    return 0;
}

/*
  Get board serial
    Tag: 0x00010004
    Request: Length: 0
    Response: Length: 8
    Value: u64: board serial
*/
uint64_t rpi_get_serial_num(void) {
    static volatile unsigned *u = 0;

    // allocate once: must be 16-byte aligned.
    if(!u)
        u = kmalloc_aligned(8*4,16);
    memset((void*)u, 0, 8*4);

    // should abstract this.
    u[0] = 6*4;     // total size in bytes.
    u[1] = 0;       // 0 = "process request"

    // serial tag
    u[2] = 0x00010004;
    u[3] = 8;   // response size size
    u[4] = 0;   // space for first 4 bytes of serial
    u[5] = 0;   // space for second 4 bytes of serial
    u[6] = 0;   // end tag

    mbox_send(MBOX_CH, u);

    if(u[1] != 0x80000000)
		panic("invalid response: got %x\n", u[1]);
    return ((uint64_t)u[6] << 32) | u[5];
}

uint32_t rpi_get_memsize(void) {
    static volatile unsigned *u = 0;

    if(!u)
        u = kmalloc_aligned(8*4,16);
    memset((void*)u, 0, 8*4);

    u[0] = 8*4;   // total size in bytes.
    u[1] = 0;   // always 0 when sending.
    // serial tag
    u[2] = 0x00010005;
    u[3] = 8;   // response size size
    u[4] = 0;   // request size
    u[5] = 0;   // space for base of mem
    u[6] = 0;   // space for sizeo of mem
    u[7] = 0;   // end tag

    mbox_send(MBOX_CH, u);

    if(u[1] != 0x80000000)
		panic("invalid response: got %x\n", u[1]);

    demand(u[5] == 0, "expected 0 base, have %d\n", u[5]);
    return u[6];
}

// hardware board model.
/*
Get board model
Tag: 0x00010001
Request:
Length: 0
Response:
Length: 4
Value:
u32: board model
*/
unsigned rpi_get_model(void) {
    static volatile unsigned *u = 0;

    if(!u)
        u = kmalloc_aligned(7*4,16);
    memset((void*)u, 0, 7*4);

    u[0] = 7*4;   // total size in bytes.
    u[1] = 0;   // always 0 when sending.
    // serial tag
    u[2] = 0x00010001;
    u[3] = 4;   // response size
    u[4] = 0;   // request size
    u[5] = 0;   // space for board #
    u[6] = 0;   // end tag

    mbox_write(MBOX_CH, u);
    mbox_read(MBOX_CH);

    if(u[1] != 0x80000000)
		panic("invalid response: got %x\n", u[1]);
    return u[5];
}

unsigned rpi_get_revision(void) {
    static volatile unsigned *u = 0;

    if(!u)
        u = kmalloc_aligned(7*4,16);
    memset((void*)u, 0, 7*4);

    u[0] = 7*4;   // total size in bytes.
    u[1] = 0;   // always 0 when sending.
    // serial tag
    u[2] = 0x00010002;
    u[3] = 4;   // response size
    u[4] = 0;   // request size
    u[5] = 0;   // space for board #
    u[6] = 0;   // end tag

    mbox_write(MBOX_CH, u);
    mbox_read(MBOX_CH);

    if(u[1] != 0x80000000)
		panic("invalid response: got %x\n", u[1]);
    return u[5];
}

#if 0
/***********************************************************************
 * trivial driver.
 */
void notmain(void) { 
    atag_print();
    unsigned size = atag_mem();
    printk("atag mem = %d (%dMB)\n\n\n", size, size / OneMB);

    unsigned u[2];
    rpi_get_serial(u);
    printk("mailbox serial = 0x%x%x\n", u[0],u[1]);

    size = rpi_get_memsize();
    printk("mailbox physical mem: size=%d (%dMB)\n", size, size/OneMB);

#if 0
    // some other useful things.
    printk("board model = 0x%x\n", rpi_get_model());
    printk("board revision= 0x%x\n", rpi_get_revision());
#endif
    clean_reboot();
}
#endif
