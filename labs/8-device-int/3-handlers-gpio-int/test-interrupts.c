// test code for checking the interrupts. 
// see <test-interrupts.h> for additional routines.
#include "test-interrupts.h"
#include "timer-interrupt.h"
#include "vector-base.h"

volatile int n_interrupt;

static interrupt_fn_t interrupt_fn;

// default vector: just forwards it to the registered
// handler see <test-interrupts.h> and the given test.
void interrupt_vector(unsigned pc) {
    dev_barrier();
    n_interrupt++;

    if(!interrupt_fn(pc))
        panic("should have no other interrupts?\n");

    dev_barrier();
}

// initialize all the interrupt stuff.  client passes in the 
// gpio int routine <fn>
//
// make sure you understand how this works.
void test_startup(init_fn_t init_fn, interrupt_fn_t int_fn) {
    output("\tImportant: must loop back (attach a jumper to) pins 20 & 21\n");
    output("\tImportant: must loop back (attach a jumper to) pins 20 & 21\n");
    output("\tImportant: must loop back (attach a jumper to) pins 20 & 21\n");

    // initialize.
    extern uint32_t interrupt_vec[];
    int_vec_init(interrupt_vec);

    gpio_set_output(out_pin);
    gpio_set_input(in_pin);

    init_fn();
    interrupt_fn = int_fn;

    // in case there was an event queued up clear it.
    gpio_event_clear(in_pin);

    // start global interrupts.
    cpsr_int_enable();
}


/********************************************************************
 * falling edge.
 */

volatile int n_falling;

// check if there is an event, check if it was a falling edge.
int falling_handler(uint32_t pc) {
    todo("implement this: return 0 if no rising int\n");
    return 0;
}

void falling_init(void) {
    gpio_write(out_pin, 1);
    gpio_int_falling_edge(in_pin);
}

/********************************************************************
 * rising edge.
 */

volatile int n_rising;

// check if there is an event, check if it was a rising edge.
int rising_handler(uint32_t pc) {
    todo("implement this: return 0 if no rising int\n");
    return 0;
}

void rising_init(void) {
    gpio_write(out_pin, 0);
    gpio_int_rising_edge(in_pin);
}

/********************************************************************
 * timer interrupt
 */

void timer_test_init(void) {
    // turn on timer interrupts.
    timer_init(1, 0x4);
}

// make sure this gets called on each timer interrupt.
int timer_test_handler(uint32_t pc) {
    dev_barrier();
    // should look very similar to the timer interrupt handler.
    todo("implement this by stealing pieces from 4-interrupts/0-timer-int");
    dev_barrier();
    return 0;
}
