#ifndef __EQX_H__
#define __EQX_H__
/*******************************************************************
 * simple process support.
 */

#include "switchto.h"   // needed for <regs_t>

#include "pinned-vm.h"
typedef struct {
    uint32_t va, pa;
    pin_t attr;
} map_t;

typedef struct eqx_th {
    // thread's registers.
    regs_t regs;

    struct eqx_th *next;

    // if non-zero: the hash we expect to get when
    // the thread exits
    uint32_t expected_hash;

    // the current cumulative hash
    uint32_t reg_hash;          

    uint32_t tid;           // thread id.

    // if we do pinned.
    map_t data_pin;
    map_t code_pin;
    
    uint32_t fn;
    uint32_t arg;
    uint32_t stack_start;
    uint32_t stack_end;
    uint32_t refork_cnt;

    // how many instructions we executed.
    uint32_t inst_cnt;
    unsigned verbose_p;  // if you want alot of information.
} eqx_th_t;

// a very heavy handed initialization just for today's lab.
// assumes it has total control of system calls etc.
void eqx_init(void);

// run all the threads until the runqueue is empty.
//  - returns xor-hash of all hashes.
uint32_t eqx_run_threads(void);


void eqx_refork(eqx_th_t *th);

eqx_th_t * eqx_fork(void (*fn)(void*), void *arg, uint32_t hash);
eqx_th_t * eqx_fork_nostack(void (*fn)(void*), void *arg, uint32_t hash) ;
eqx_th_t * eqx_fork_stack(void (*fn)(void*), void *arg,
    uint32_t expected_hash, void *stack, uint32_t nbytes);

// 1 = chatty, 0 = quiet
// note: should have more levels so can be precise about
//       logging.
extern int eqx_verbose_p;
static inline void eqx_verbose(int v_p)
    { eqx_verbose_p = v_p; }


// default stack size.
enum { eqx_stack_size = 8192 * 8 };
_Static_assert(eqx_stack_size > 1024, "too small");
_Static_assert(eqx_stack_size % 8 == 0, "not aligned");


// called by client code.
void sys_equiv_exit(uint32_t ret);

#endif
