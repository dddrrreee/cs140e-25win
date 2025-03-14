// run threads all at once: should see their pids smeared.
#include "rpi.h"
#include "eqx-os.h"
#include "expected-hashes.h"
#include "user-progs/byte-array-0-hello.h"

void sys_equiv_putc(uint8_t ch);

void notmain(void) {
    eqx_verbose(1);


    eqx_config_t c = {
        .ramMB = 265,
        .vm_use_pin_p = 1
    };
    eqx_init_config(c);

    // in user-progs.
    struct prog *p = &bytes_0_hello;
    output("about to load: <%s>\n", p->name);

    let th = eqx_exec_internal(p,  0);

    output("about to run\n");
    eqx_run_threads();
    
    eqx_exec_internal(p,  th->expected_hash);
    eqx_exec_internal(p,  th->expected_hash);
    eqx_exec_internal(p,  th->expected_hash);
    eqx_exec_internal(p,  th->expected_hash);
    eqx_exec_internal(p,  th->expected_hash);
    eqx_exec_internal(p,  th->expected_hash);
    eqx_exec_internal(p,  th->expected_hash);
    eqx_exec_internal(p,  th->expected_hash);
    eqx_run_threads();
}
