// driver to see if relocation works; tests speedup.
#include "rpi.h"
#include "rpi-inline-asm.h" // libpi/include
#include "cpsr-util.h"      // libpi/include

// both routines are in <load-state-asm.S>
void rfe_user(uint32_t regs[4]);
void check_rfe_asm(void);

// test that we get what we expect when we use the SRS 
// instruction.
void test_srs(void) {
    // in asm.S
    void srs_super(uint32_t r[2], uint32_t *lr);

    uint32_t srs_r[2] = {0}, lr = 0, spsr = spsr_get();

    // use the SRS instruction to save the {lr,spsr} of super mode.
    srs_super(srs_r, &lr);
    printk("spsr = %x, expected %x, r14=%x, expected %x\n", 
            srs_r[1], spsr, lr, srs_r[0]);
    assert(srs_r[1] == spsr);
    assert(srs_r[0] == lr);
    trace("test_srs: passed!\n");
}

static unsigned expect_sp, expect_a0, expect_lr;

// check that rfe works: lr, sp, and a0 are all what
// we expect
void check_rfe(unsigned a0, unsigned sp, unsigned lr) {
    debug("check_rfe: cpsr=%s\n", mode_str(cpsr_get()));
    output("\thave:     a0=%x sp=%x lr=%x\n", a0, sp,lr);
    output("\texpected: a0=%x sp=%x lr=%x\n", 
                            expect_a0, expect_sp,expect_lr);

    assert(a0 == expect_a0);
    assert(sp == expect_sp);
    assert(lr == expect_lr);
    assert(mode_get(cpsr_get()) == USER_MODE);

    printk("SUCCESS:check_rfe\n");
    // should not allow user to reboot the machine :)   TODO
    clean_reboot();
}

void notmain(void) {
    debug("mode=%s\n", mode_str(cpsr_get()));
    test_srs();

    // we do this to sort of mirror the equivalance code ---
    // higher number registers at higher addresses
    uint32_t regs[4];
    expect_a0 = (uint32_t)regs;
    expect_sp = regs[0] = STACK_ADDR2;      // user sp
    expect_lr = regs[1] = 0xdeadbeef;       // user lr
    regs[2] = (uint32_t)check_rfe_asm;      // user pc
    regs[3] = 0x190;                        // user cpsr
    rfe_user(regs);
    not_reached();
}

void impossible(uint32_t pc) {
    panic("impossible: %x!\n", pc);
}
