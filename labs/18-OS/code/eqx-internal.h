#ifndef __EQX_INTERNAL_H__
#define __EQX_INTERNAL_H__

#define eqx_trace(args...) do {                                 \
    if(eqx_verbose_p) {                                         \
        printk("TRACE:%s:", __FUNCTION__); printk(args);    \
    }                                                       \
} while(0)

// utility routine to print the non-zero registers.
//
//  - you probably want to mess w/ the output formatting.
static inline void reg_dump(int tid, int cnt, regs_t *r) {
    if(!eqx_verbose_p)
        return;

    uint32_t pc = r->regs[REGS_PC];
    uint32_t cpsr = r->regs[REGS_CPSR];
    output("tid=%d: pc=%x cpsr=%x: ",
        tid, pc, cpsr);
    if(!cnt) {
        output("  {first instruction}\n");
        return;
    }

    int changes = 0;
    output("\n");
    for(unsigned i = 0; i<15; i++) {
        if(r->regs[i]) {
            output("   r%d=%x, ", i, r->regs[i]);
            changes++;
        }
        if(changes && changes % 4 == 0)
            output("\n");
    }
    if(!changes)
        output("  {no changes}\n");
    else if(changes % 4 != 0)
        output("\n");
}

#endif
