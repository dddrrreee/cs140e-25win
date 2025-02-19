// simple single-step threads package that use mismatching
// to compute a running hash of each threads registers.
// great for detecting errors.
//
// once this works, you should be able to tweak / add 
// functionality and it will detect if anything went wrong.
//
// this is roughly an expansion of the check-interleave
// lab's code, but without the systematic interleaving.
// given N threads, the default code:
//  - runs thread 1 for one instruction; 
//  - switches and runs thread 2 for one instruction;
//  - switches and runs thread 3 for one instruction;
//  - etc.
//  - as each thread exits it compares the computed hash to
//    the expected hash.
//
// most the of the code you've seen before in different 
// labs:
//  - setting up and handling a single step exception
//  - doing system calls
//  - setting up threads using <reg_t> 
//  - yielding from one thread to another.
//  - about 1/3 should be new.
#include "rpi.h"
#include "eqx-threads.h"
#include "breakpoint.h"
#include "full-except.h"
#include "fast-hash32.h"
#include "eqx-syscalls.h"
#include "cpsr-util.h"

// check for initialization bugs.
static int eqx_init_p = 0;

static int verbose_p = 1;
void eqx_verbose(int v_p) {
    verbose_p = v_p;
}
#define eqx_trace(args...) do {                                 \
    if(verbose_p) {                                         \
        printk("TRACE:%s:", __FUNCTION__); printk(args);    \
    }                                                       \
} while(0)

// simple thread queue.
//  - should make so you can delete from the middle.
typedef struct rq {
    eqx_th_t *head, *tail;
} rq_t;

// will define eqx_pop, eqx_push, eqx_append, etc
#include "queue-ext-T.h"
gen_queue_T(eqx_th, rq_t, head, tail, eqx_th_t, next)
static rq_t eqx_runq;
static uint32_t exit_hash = 0;

// pointer to current thread.  not null when running
// threads, null when not.
static eqx_th_t * volatile cur_thread;

// just like in the interleave lab (10): where we switch back to.
static regs_t start_regs;

// check that the <sp> stack pointer reg is within 
// the thread stack.  just die if not.  (should
// do something else!).
static int eqx_check_sp(eqx_th_t *th) {
    let sp = th->regs.regs[REGS_SP];
    if(sp < th->stack_start)
        panic("stack is too small: %x, lowest legal=%x\n",
            sp, th->stack_start);
    if(sp > th->stack_end)
        panic("stack is too high: %x, highest legal=%x\n",
            sp, th->stack_end);
    return 1;
}

// internal helper.  matches the thread initialization
// in our check-interleave checker.
//
// initialize <th> registers to call <th->fn(th->arg)>
// and, if that returns call <sys_equiv_exit>. 
//
// computes the threads initial <cpsr> from the current 
// <cpsr>.
static void eqx_regs_init(eqx_th_t *th) {
    // calculate thread <cpsr>
    //  1. default: get the current <cpsr>, so that we keep
    //    the interrupt status etc) and:
    //  2. clear the carry flags since they have nothing
    //     to do with the thread.
    //  3. set the mode to user so can single step.
    // caller can change this when we return if desired.
    uint32_t cpsr = cpsr_inherit(USER_MODE, cpsr_get());

    // initialize the thread block 
    //  see <switchto.h>
    regs_t r = {
        .regs[REGS_PC] = (uint32_t)th->fn,
        // the first argument to fn
        .regs[REGS_R0] = (uint32_t)th->arg,

        // stack pointer register
        .regs[REGS_SP] = (uint32_t)th->stack_end,
        // the cpsr to use
        .regs[REGS_CPSR] = cpsr,

        // where to jump to if the code returns.
        .regs[REGS_LR] = (uint32_t)sys_equiv_exit,
    };

    th->regs = r;
    if(!eqx_check_sp(th))
        panic("stack is out of bounds!\n");
}


// fork <fn(arg)> as a pre-emptive thread and allow the caller
// to allocate the stack.
//   - <expected_hash>: if non-zero gives the expected ss hash.
//   - <stack>: the base of the stack (8 byte aligned).
//   - <nbytes>: the size of the stack.
eqx_th_t *
eqx_fork_stack(void (*fn)(void*), void *arg, 
    uint32_t expected_hash,
    void *stack, 
    uint32_t nbytes)  {

    eqx_th_t *th = kmalloc(sizeof *th);
    th->fn  = (uint32_t)fn;
    th->arg = (uint32_t)arg;
    th->expected_hash = expected_hash;

    // we do a dumb monotonic thread id [1,2,...]
    static unsigned ntids = 1;
    th->tid = ntids++;

    // stack grows down: must be 8-byte aligned.
    th->stack_start = (uint32_t)stack;
    th->stack_end = th->stack_start + nbytes;
    // check stack alignment.
    unsigned rem = (uint32_t)th->stack_end % 8;
    if(rem)
        panic("stack is not 8 byte aligned: mod 8 = %d\n", rem);

    eqx_regs_init(th);
    eqx_th_push(&eqx_runq, th);
    return th;
}

// fork + allocate a 8-byte aligned stack.
eqx_th_t *
eqx_fork(void (*fn)(void*), void *arg, 
    uint32_t expected_hash) {

    void *stack = kmalloc_aligned(eqx_stack_size, 8);
    assert((uint32_t)stack%8==0);

    return eqx_fork_stack(fn, arg, 
                expected_hash, 
                stack, eqx_stack_size);
}

// fork with no stack: this is used as a debugging
// aid to support the easiest case of routines
// that just do ALU operations and do not use a stack.
eqx_th_t * 
eqx_fork_nostack(void (*fn)(void*), void *arg, 
    uint32_t expected_hash) 
{
    return eqx_fork_stack(fn,arg,expected_hash,0,0);
}

// given a terminated thread <th>: reset its
// state so it can rerun.  this is used to 
// run the same thread in different contexts
// and make sure we get the same hash.
void eqx_refork(eqx_th_t *th) {
    // should be computed by sys_exit()
    assert(th->expected_hash);

    eqx_regs_init(th);

    // reset our counts and running hash.
    th->inst_cnt = 0;
    th->reg_hash = 0;

    eqx_th_push(&eqx_runq, th);
}

// run a single instruction by setting a mismatch on the 
// on pc.  we make sure the uart can accept a character
// so that we can guard against UART race conditions.
//
//  - this does not return to the caller.
static __attribute__((noreturn))
void brkpt_run_one_inst(regs_t *r) {
    brkpt_mismatch_set(r->regs[REGS_PC]);

    // otherwise there is a race condition if we are 
    // stepping through the uart code --- note: we could
    // just check the pc and the address range of
    // uart.o
    //
    // note this is the same as lab 10.
    while(!uart_can_put8())
        ;

    switchto(r);
}

// pick the next thread and run it.
//
// currently just does round robin by popping the next
// thread off the run queue, but can do anything you
// want.
//
//  - this does not return to the caller.
static __attribute__((noreturn)) void 
eqx_schedule(void) 
{
    assert(cur_thread);

    eqx_th_t *th = eqx_th_pop(&eqx_runq);
    if(th) {
        if(th->verbose_p)
            output("switching from tid=%d,pc=%x to tid=%d,pc=%x,sp=%x\n",
                cur_thread->tid,
                cur_thread->regs.regs[REGS_PC],
                th->tid,
                th->regs.regs[REGS_PC],
                th->regs.regs[REGS_SP]);
        // put the current thread at the end of the runqueue.
        eqx_th_append(&eqx_runq, cur_thread);
        cur_thread = th;
    }
    brkpt_run_one_inst(&cur_thread->regs);
}


// utility routine to print the non-zero registers.
//
//  - you probably want to mess w/ the output formatting.
static void reg_dump(int tid, int cnt, regs_t *r) {
    if(!verbose_p)
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

// single-step exception handler.  similar to the lab 10
// check-interleave.c handler in that it sets breakpoints
// and catches them.
//
// will detect with reasonable probability if even a single 
// register differs by a single bit at any point during a 
// thread's execution as compared to previous runs.
//
// is simpler than <check-interleave.c> in that it just 
// computes a 
//   - hash of all the registers <regs>;
//   - combines this with the current thread hash <th->reg_hash>.
// at exit: this hash will get compared to the expected hash.
//
static void equiv_single_step_handler(regs_t *regs) {
    if(!brkpt_fault_p())
        panic("impossible: should get no other faults\n");

    let th = cur_thread;
    assert(th);

    // copy the saved registers <regs> into thread the
    // <th>'s register block.
    //  - we could be more clever and save them into <th>->regs
    //    in the first place.
    th->regs = *regs;   

    // increment for each instruction.
    th->inst_cnt++;     

    // if you need to debug can drop this in to see the values of
    // the registers.
    // reg_dump(th->tid, th->inst_cnt, &th->regs);

    uint32_t pc = regs->regs[15];

    // need to do so it relabels the cpsr: 
    //  if(cpsr.mode!=x)
    //      panic()
    //  else
    //      cpsr.mode = USER
    //  then the hash works and you don't have to keep around all
    //  the different ones.
    //
    // have them do system calls?
    th->reg_hash = fast_hash_inc32(&th->regs, sizeof th->regs, th->reg_hash);

    // should let them turn it off.
    if(th->verbose_p)
        output("hash: tid=%d: cnt=%d: pc=%x, hash=%x\n",
            th->tid, th->inst_cnt, pc, th->reg_hash);

    eqx_schedule();
}


// our two system calls: 
//   - exit: get the next thread if there is one.
//   - putc: so we can handle race conditions with prints
static int equiv_syscall_handler(regs_t *r) {
    // sanity checking.
    //  - mode routines in <libpi/include/cpsr-util.h>
    //
    // 1. the SPSR better = what we saved!
    uint32_t spsr = spsr_get();
    assert(r->regs[REGS_CPSR] == spsr);
    // 2. it better be either USER or (later) SYSTEM
    assert(mode_get(spsr) == USER_MODE);
    // 3. since running, should have non-null <cur_thread>
    let th = cur_thread;
    assert(th);

    // update the registers
    th->regs = *r;  

    // check the stack.
    eqx_check_sp(th);

    unsigned sysno = r->regs[0];
    switch(sysno) {
    case EQX_SYS_PUTC:
        uart_put8(r->regs[1]);
        break;
    case EQX_SYS_EXIT:
        eqx_trace("thread=%d exited with code=%d, hash=%x\n",
            th->tid, r->regs[1], th->reg_hash);

        // if we don't have a hash, add it from this run.  
        // ASSUMES: that the code is deterministic so will 
        // compute the same hash on the next run.  this 
        // obviously is a strong assumption.
        if(!th->expected_hash)
            th->expected_hash = th->reg_hash;
        // if we do have an expected hash, check that it matches.
        else if(th->expected_hash) {
            let exp = th->expected_hash;
            let got = th->reg_hash;
            if(exp == got) {
                eqx_trace("EXIT HASH MATCH: tid=%d: hash=%x\n",
                    th->tid, exp, got);
            } else {
                panic("MISMATCH ERROR: tid=%d: expected hash=%x, have=%x\n",
                    th->tid, exp, got);
            }
        }
        // add to all the other thread hashes. 
        // communative but weak.
        exit_hash += th->reg_hash;

        // if no more threads on the runqueue we are done.
        if(!(cur_thread = eqx_th_pop(&eqx_runq))) {
            eqx_trace("done with all threads\n");
            switchto(&start_regs);
        }

        // otherwise do the next one.
        brkpt_run_one_inst(&cur_thread->regs);
        not_reached();

    default:
        panic("illegal system call: %d\n", sysno);
    }

    // pick what to run next.
    eqx_schedule();
    not_reached();
}

// one time initialization.  
//  - setup heap if haven't.
//  - install exception handlers [this is overly
//    self-important: in a real system there could
//    be other subsystems that want to do so]
void eqx_init(void) {
    if(eqx_init_p)
        panic("called init twice!\n");
    eqx_init_p = 1;

    // bad form for us to do this.   for today...
    if(!kmalloc_heap_start())
        kmalloc_init();

    // install is idempotent if already there.
    full_except_install(0);

    // for breakpoint handling (like lab 10)
    full_except_set_prefetch(equiv_single_step_handler);
    // for system calls (like many labs)
    full_except_set_syscall(equiv_syscall_handler);
}

// run all threads in the run-queue using 
// single step mode.   
//   - should make a non-ss version.
//   - make sure you can add timer interrupts.
//   - make sure you can do match faults.
uint32_t eqx_run_threads(void) {
    if(!eqx_init_p)
        panic("did not initialize eqx!\n");

    exit_hash = 0;
    cur_thread = eqx_th_pop(&eqx_runq);
    // for today we don't expect an empty runqueue,
    // but you can certainly get rid of this if prefer.
    if(!cur_thread)
        panic("empty run queue?\n");

    // start mismatching.  
    brkpt_mismatch_start();

    // note: we're running the first instruction and 
    // *then* getting a mismatch.
    brkpt_mismatch_set(cur_thread->regs.regs[15]);

    // just like check-interleave.c: cswitch so can
    // come back here.  nothing else better use our
    // current stack!!
    switchto_cswitch(&start_regs, &cur_thread->regs);

    // done mismatching.
    brkpt_mismatch_stop();

    // check that runqueue empty.
    cur_thread = eqx_th_pop(&eqx_runq);
    if(cur_thread)
        panic("run queue should be empty\n");

    // better be empty.
    assert(!eqx_runq.head);
    assert(!eqx_runq.tail);

    eqx_trace("done running threads\n");

    return exit_hash;
}
