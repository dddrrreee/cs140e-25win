#ifndef __CPSR_UTIL_H__
#define __CPSR_UTIL_H__
// utulities work on spsr and cpsr

#include "rpi-interrupts.h"  // for cpsr_get, cpsr_set
#include "rpi-constants.h"  // for the modes.
#include "libc/bit-support.h"
#include "rpi-inline-asm.h"

// cpsr and spsr
static inline int mode_legal(unsigned mode) {
    if(mode == USER_MODE
        || mode == IRQ_MODE
        || mode == SUPER_MODE
        || mode == ABORT_MODE
        || mode == UNDEF_MODE
        || mode == SYS_MODE)
            return 1;
    panic("illegal mode: %x [bits=%b]\n", mode, mode);
}

static inline uint32_t 
mode_get(uint32_t cpsr) {
    unsigned mode = bits_get(cpsr, 0, 4);
    assert(mode_legal(mode));
    return mode;
}

static inline uint32_t 
mode_set(uint32_t cpsr, unsigned mode) {
    assert(mode_legal(mode));
    return bits_clr(cpsr, 0, 4) | mode;
}

// works on spsr as well.
static inline uint32_t 
cpsr_clear_carry(uint32_t cpsr) {
    return bits_clr(cpsr, 28, 31);
}

static inline const char *mode_str(uint32_t cpsr) {
    switch(mode_get(cpsr)) {
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

// get the saved status register.
static inline uint32_t spsr_get(void) {
    uint32_t spsr = 0;
    asm volatile("mrs %0,spsr" : "=r"(spsr));
    return spsr;
}

// change the mode field mode in <cpsr> to <mode>: 
// clear the carry flags, keep
// keep interrupts, thumb, etc.
static inline uint32_t cpsr_inherit(uint32_t mode, uint32_t cpsr) {
    assert(mode_legal(mode));
    return mode_set(cpsr_clear_carry(cpsr), mode);
}

static inline int mode_eq(uint32_t mode) { return mode_get(cpsr_get()) == mode; }
static inline int mode_is_super(void) { return mode_eq(SUPER_MODE); }
static inline int mode_is_user(void) { return mode_eq(USER_MODE); }

#endif
