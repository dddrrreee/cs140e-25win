// example of how to get mode: you don't need so much stuff.
#include "rpi.h"

// get the status register: example of inline assembly.
// could also put in .S file.
static inline uint32_t cpsr_get(void) {
    uint32_t cpsr;
    asm volatile("mrs %0,cpsr" : "=r"(cpsr));
    return cpsr;
}

// libpi/include/rpi-constants.h defines the
// *_MODE constants
const char *mode_get(void) {
    uint32_t cpsr = cpsr_get();
    uint32_t mode = cpsr & 0b11111;

    switch(mode) {
    case USER_MODE: return "USER MODE";
    case FIQ_MODE:  return "FIQ MODE";
    case IRQ_MODE:  return "IRQ MODE";
    case SUPER_MODE: return "SUPER MODE";
    case ABORT_MODE: return "ABORT MODE";
    case UNDEF_MODE: return "UNDEF MODE";
    case SYS_MODE:   return "SYS MODE";
    default: panic("unknown mode: <%x>\n", cpsr);
    }
}

void notmain(void) {
    const char *mode = mode_get();
    output("starting at mode <%s>\n", mode);
}
