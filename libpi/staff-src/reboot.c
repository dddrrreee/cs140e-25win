// engler, cs140e: reboot the pi.
#include "rpi.h"

void rpi_reboot(void) {
//    uart_flush_tx();
    delay_ms(10);

    // is there a way to speed this up?
    const int PM_RSTC = 0x2010001c;
    const int PM_WDOG = 0x20100024;
    const int PM_PASSWORD = 0x5a000000;
    const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

    // timeout = 1/16th of a second? (whatever)
    PUT32(PM_WDOG, PM_PASSWORD | 1);
    PUT32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
    while(1); 
}
