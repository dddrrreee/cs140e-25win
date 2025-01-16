#include "rpi.h"
#include "cycle-count.h"
#include "memmap.h"

// called to setup the C runtime and initialize commonly
// used subsystems.
void _cstart() {
	void notmain();

    gcc_mb();
    uint8_t * bss      = (void*)__bss_start__;
    uint8_t * bss_end  = (void*)__bss_end__;
    while( bss < bss_end )
        *bss++ = 0;
    gcc_mb();

    // currently default init to 115200 baud.
    uart_init();

    // have to initialize the cycle counter or it returns 0.
    // i don't think any downside to doing here.
    cycle_cnt_init();

    // should add argv.
    notmain(); 
	clean_reboot();
}
