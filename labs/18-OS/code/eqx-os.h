#ifndef __EQX_OS_H__
#define __EQX_OS_H__

#include "rpi.h"
#include "eqx-threads.h"

// bundle all the configuration stuff in one
// structure
typedef struct {
#if 0
    // if != 0: randomly flip a coin and 
    // Pr(1/random_switch) switch to another 
    // process in ss handler.
    uint32_t random_switch;

    // if 1: turn on cache [this will take work]
    unsigned do_caches_p:1;

    // if 1: turn vm off and on in the ss handler.
    unsigned do_vm_off_on:1;

    // if 1, randomly add to head or tail of runq.
    unsigned do_random_enq:1;

    unsigned emit_exit_p:1; // emit stats on exit.

    unsigned no_vm_p:1; // no VM
#endif
#if 0
    unsigned verbose_p:1,
             // if you run w/ icache on: have to flush the data cache and icache
             // when modify code.
             icache_on_p:1,
             compute_hash_p:1,
             vm_off_p:1,
             run_one_p:1,
             hash_must_exist_p:1,
             disable_asid_p:1;

    unsigned debug_level;     // can be set w/ a runtime config.
    unsigned ramMB;           // default is 128MB
#endif
    // verbose debugging info.
    unsigned verbose_p:1,
             no_user_access_p:1,

             // exclusive: use pin, use pt, or use nothing.
             vm_off_p:1,    // implies the others are off.
             vm_use_pin_p:1,
             vm_use_pt_p:1

            ;
    unsigned ramMB;           // default is 128MB

    unsigned user_idx;
} eqx_config_t;

extern eqx_config_t eqx_config;
void eqx_init_config(eqx_config_t c);

#include "small-prog.h"
eqx_th_t* eqx_exec_internal(struct prog *prog, uint32_t expected_hash);


#endif
