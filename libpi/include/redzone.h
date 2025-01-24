#ifndef __REDZONE_H__
#define __REDZONE_H__

// simple-stupid way to check memory corruption in the
// address range [0..4096)
//
// 1. set [0..4096) to 0.
// 2. check that its still 0 when called.
// 
// if this ever fails you are probably writing to null.
enum { RZ_NBYTES = 4096 };

static inline int redzone_check(const char *msg) {
    volatile uint32_t *rz = (void*)0;

    if(msg)
        output("redzone checking: %s\n", msg);

    unsigned nfail = 0;
    for(unsigned i = 0; i < RZ_NBYTES/4; i++) {
        if(rz[i] != 0) {
            nfail++;
            output("non-zero redzone: off=%x, val=%x fail=%d\n", i*4,rz[i], nfail);
        }
    }
    if(nfail)
        panic("redzone: total failures =%d\n", nfail);

    return nfail == 0;
}

static inline void redzone_init(void) {
    memset(0, 0, RZ_NBYTES);
    assert(redzone_check(0));
}
#endif
