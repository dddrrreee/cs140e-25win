#ifndef __RPI_CONFIG_H__
#define __RPI_CONFIG_H__

// start adding rpi configuration flags here.

// if this is defined, we use uart_flush_tx rather than delaying for a hacked
// number of milliseconds.
#define RPI_REBOOT_FLUSH_TX

// baud rate on the pi side.
#define RPI_UART_BAUD 115200
//#define RPI_UART_BAUD 921600

#endif
