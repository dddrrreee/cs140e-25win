## checking that libpi is not broken.

These are simple checks to make sure libpi is still kinda working.
  1. Before updating libpi to use your gpio.c, run them to see that they
     work.
  2. Copy `gpio.c` to `libpi/src/gpio.c` and update the `libpi/Makefile`.
     `hello.bin` should print hello, and `act-led.bin` should blink the ACT led
     on the pi.
  3. Rerun these and see that they work.
