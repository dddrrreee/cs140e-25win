#include "rpi.h"

// unique, we leave with defines so they can be used in assembly.
#define SYS_EXIT 1
#define SYS_PUTC 2

int handle_swi_full(int sysno, uint32_t a0, uint32_t a1, uint32_t *regs) {
    printk("sysno=%d\n", sysno);
    switch(sysno) {
    case SYS_PUTC: 
        printk("%c", a0); 
        return a0;
    case SYS_EXIT: clean_reboot();
    default: panic("invalid sysno: %d!\n", sysno);
    }
}

