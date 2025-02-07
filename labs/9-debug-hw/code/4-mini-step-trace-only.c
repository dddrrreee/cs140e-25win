#include "rpi.h"
#include "mini-step.h"

static int cnt;

// just print out the pc and instruction count.
static void step_handler_trivial(void *data, step_fault_t *s) {
    let regs = s->regs->regs;
    uint32_t pc = regs[15];
    output("TRACE: cnt=%d: pc=%x\n", cnt, pc);
}

void nop_10(void);
void mov_ident(void);

void notmain(void) {
    kmalloc_init();

    output("---------------------------------------------\n");
    output("going to run trivial handler\n");

    mini_step_init(step_handler_trivial, 0);
    output("about to run nop10()!\n");
    cnt = 0;
    mini_step_run((void*)nop_10, 0);
    output("done nop10()!\n");

    output("about to run mov_ident()!\n");
    cnt = 0;
    mini_step_run((void*)mov_ident, 0);
    output("done mov_ident()!\n");


    clean_reboot();
}
