// roughly measure how many cycles pass in one second.
// worth experimenting to get a cleaner reading!
#include "rpi.h"
#include "cycle-count.h"

void notmain(void) {
    unsigned cyc_start = cycle_cnt_read();

    unsigned s = timer_get_usec();
    while((timer_get_usec() - s) < 1000*1000)
        ;
    unsigned cyc_end = cycle_cnt_read();

    unsigned tot = cyc_end - cyc_start;
    printk("cycles per sec = %d\n", tot);

    unsigned exp = 700 * 1000 * 1000;
    printk("expected - measured = %d cycles of error\n", exp-tot);
}
