#include "rpi.h"
#include "mini-step.h"

static regs_t last_regs;
static int cnt;

// print the register diferrences
static void step_handler_diff(void *data, step_fault_t *s) {
    let regs = s->regs->regs;
    uint32_t pc = regs[15];
    output("TRACE: cnt=%d: pc=%x:", cnt, pc);
    if(!cnt) {
        output("  {first instruction}\n");
    } else {
        int changes = 0;
        // ignore cpsr and pc
        for(unsigned i = 0; i<15; i++) {
            let new = regs[i];
            let old = last_regs.regs[i];
            if(old != new) {
                if(!changes)
                    output("\n");
                output("TRACE:\tr%d changed: old=%x, new=%x\n", i, old,new);
                changes++;
            }
        }
        if(!changes)
            output("  {no changes}\n");
            
    
    }
    last_regs = *s->regs;
    cnt++;
}

void nop_10(void);
void mov_ident(void);

void notmain(void) {
    kmalloc_init();

    trace("---------------------------------------------\n");
    trace("going to run difference handler\n");

    mini_step_init(step_handler_diff, 0);
    trace("about to run nop10()!\n");
    cnt = 0;
    mini_step_run((void*)nop_10, 0);
    trace("done nop10()!\n");

    trace("about to run mov_ident()!\n");
    cnt = 0;
    mini_step_run((void*)mov_ident, 0);
    trace("done mov_ident()!\n");


    clean_reboot();
}
