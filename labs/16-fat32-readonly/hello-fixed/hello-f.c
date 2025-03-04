#include "rpi.h"

void notmain(void) {
	// NB: we can't do this b/c the shell already initialized and resetting
	// uart may reset connection to Unix.
	// uart_init();

	// if not working, try just printing characters.
#if 0
	uart_putc('h');
	uart_putc('e');
	uart_putc('l');
	uart_putc('l');
	uart_putc('o');

	uart_putc(' ');

	uart_putc('w');
	uart_putc('o');
	uart_putc('r');
	uart_putc('l');
	uart_putc('d');

	uart_putc('\n');
#endif

    // <_start> is defined in <loader-start.S>
    extern uint32_t _start[];
	printk("TRACE: hello world from address %p\n", (void*)_start);
	return;

	// NB: this is supposed to be a thread_exit().  calling reboot will
	// kill the pi.
	// clean_reboot();
}
