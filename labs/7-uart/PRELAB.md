### Prelab: UART

The UART device (`uart.o`) is the last magic bit of low level code.
It's what you've used for bootloading on the pi-side in the last lab and
what `printk` and friends use for output.  In this lab you'll write the
driver for it.  At this point, all the interesting code will be yours.

UART isn't much code, but it is extremely hard to debug if you get it
wrong.  Since `printk` uses UART, a UART bug means `printk` won't work.
If `printk` doesn't work you can't get any information out of the pi
other than silence --- not even the crash location, much less the values.

Thus, you should do the readings very carefully.  The less you understand
the device, the more bugs you'll make and the longer they will take. We
suggest reading through the broadcom pages (about 13 pages, below)
at least 3 times with notes.    Also go through the lab README to get a feel
for the stuff you need.

The reading:
      - [miniUART](../../notes/devices/miniUART.md) cheat sheet.
      - Broadcom document:  Sections 1, 2, 2.1, 2.2, 6.2 (p 102).  Main
        reading is pages 8---19.
      - [errata](https://elinux.org/BCM2835_datasheet_errata) has the
        usual errata.
      - [UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter)
