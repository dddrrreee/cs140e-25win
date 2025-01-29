/*                  DO NOT MODIFY THIS CODE.
 *                  DO NOT MODIFY THIS CODE.
 *                  DO NOT MODIFY THIS CODE.
 *                  DO NOT MODIFY THIS CODE.
 *
 * engler, cs140e: trivial driver for your get_code implementation,
 * which does the actual getting and loading of the code.
 */
#include "rpi.h"

/*******************************************************
 * UART implementation of our routines.
 */

// non-blocking: returns 1 if there is data, 0 otherwise.
static inline int boot_has_data(void) {
    return uart_has_data();
}

// returns 8-bits from the network connection.
//
// should probably allow this to return a failure code
// (timeout, network error)
//
// can check by making sure you get a get_code() < 0 
// (not reboot, not lockup) if there is an error.
static inline uint8_t boot_get8(void) {
    return uart_get8();
}

// sends 8-bits on the network connection.
//
// should probably allow this to return a failure code
// (timeout, network error)
// 
// can check by making sure you get a get_code() < 0 
// (not reboot, not lockup) if there is an error.
static void boot_put8(uint8_t x) {
    uart_put8(x);
}

#include "get-code.h"

void notmain(void) {
    uint32_t addr = get_code();
    if(!addr)
        rpi_reboot();

    // blx to addr.  
    // could also call it as a function pointer.
    BRANCHTO(addr);
    not_reached();
}
