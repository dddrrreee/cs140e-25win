#include "rpi.h"

void foo(void) {
    *(volatile unsigned *)0 = 12;
}

void notmain(void) {
    *(volatile unsigned *)0 = 12;

    printk("0 = %d\n", *(volatile unsigned *)0);
    printk("hello world\n");
    clean_reboot();
}
