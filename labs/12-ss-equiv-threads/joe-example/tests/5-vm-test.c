// run threads all at once: should see their pids smeared.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"
#include "pinned-vm.h"
#include "mmu.h"
#include "memmap-default.h"


void sys_equiv_putc(uint8_t ch);

// print everyting out
void equiv_puts(char *msg) {
    for(; *msg; msg++)
        sys_equiv_putc(*msg);
}

// simple thread that just prints its argument as a string.
void msg_fn(void *msg) {
    equiv_puts(msg);
}

void notmain(void) {
#if 1
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));

    // default domain bits.
    pin_mmu_init(dom_bits);

    unsigned idx = 0;
    pin_t kern = pin_mk_global(dom_kern, user_access, MEM_uncached);
    pin_mmu_sec(idx++, SEG_CODE, SEG_CODE, kern);
    pin_mmu_sec(idx++, SEG_HEAP, SEG_HEAP, kern);
    pin_mmu_sec(idx++, SEG_STACK, SEG_STACK, kern);
    pin_mmu_sec(idx++, SEG_INT_STACK, SEG_INT_STACK, kern);

    // use 16mb section for device.
    pin_t dev  = pin_16mb(pin_mk_global(dom_kern, user_access, MEM_device));
    pin_mmu_sec(idx++, SEG_BCM_0, SEG_BCM_0, dev);

    pin_set_context(1);
    pin_mmu_enable();

#endif

    eqx_verbose(0);
    eqx_init();
    
    enum { N = 8 };
    eqx_th_t *th[N];

    // run each thread by itself.
    for(int i = 0; i < N; i++) {
        void *buf = kmalloc(256);
        snprintk(buf, 256, "hello from %d\n", i+1);
        th[i] = eqx_fork(msg_fn, buf, 0);
        th[i]->verbose_p = 0;
        eqx_run_threads();
    }

    output("---------------------------------------------------\n");
    output("about to do quiet run\n");
    eqx_verbose(0);

    // refork and run all together.
    for(int i = 0; i < N; i++)
        eqx_refork(th[i]);
    eqx_run_threads();
}
