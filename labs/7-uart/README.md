## Lab: write your own UART implementation.

<p align="center">
  <img src="images/uart-meme.jpg" width="450" />
</p>


By the end of this lab, you'll have written your own device driver for
the pi's mini-UART hardware, which is what communicates with the TTY-USB
device you plug into your laptop.  This driver is the last "major" piece
of the pi runtime that you have not built yourself.  At this point, you
can print out the entire system, look at each line of code, and should
hopefully know why it is there and what it is doing.  Importantly you have
reached that magic point of understanding where: there is nothing else.

In addition, you'll implement your own *software* UART implementation
that can entirely bypass the pi hardware and talk directly to the
tty-USB device itself.  I.e., without using the driver above, but instead
building an 8n1 serial protocol with the CP2102 device over two GPIO pins.
Multiple reasons:

   1. The most practical reason: this gives you a "console"
      `printk` you can use when the UART is occupied, which turns out to
      be surprisingly useful.  For example, when you write the UART driver
      above, you can't actually use `printk` to debug it, since doing so
      requires a UART!  As another, when we use the ESP8266 networking
      device in a couple of labs: it needs a UART to communicate with
      the pi.  Without a second UART, we would have no way to print to
      the laptop.  Again: hard to debug if you have no idea what the pi
      is doing.

   2. Gives you an existence proof that you don't need to need to use
      hardware but can roll your own.  This applies to all the different
      hardware protocols  built-in to the pi, such as I2C and SPI.  These
      acronyms may sound ominous, but their implementation is not: just 
      raise or lower pins,  possibly with a time delay.

   3. Writing your own version of a hardware protocol gives you more
      of an intuition for the problems that happen at this level.
      For example, how do you deal with concurrency between two entirely
      separate devices when we don't have the usual fall-back on locks,
      etc?  (Hint: the use of messages and very precise time deadlines
      are fairly common.)

   4. This gives you a small demonstration of how your pi setup,
      which appears trivial, is a hard-real time system, where with
      some care, it possible to hit nanosecond deadlines reliably.
      Difficult to do this on a "real" system such as MacOS or Linux.

### Checkoff

You can do either part 1 (writing a software UART put8) or part 2 (doing
the hardware UART) first.  Doing the SW-uart will make debugging easier,
but if you want to play in hard mode, doing the hardware UART without
any help is historically accurate.

Show that:
   1. You have a quick way of switching directories, especially switching to
      libpi and back.  Try setting up aliases based on the `pushd` and `popd`
      commands.  Don't keep re-typing-out `cd ../../../libpi` and `cd
      ../labs/8-uart/whatever`; that's a waste of your lab time.
  
   2. `2-uart`: `make checkoff` passes.  You should first ensure you
       can pass the tests one by one to make debugging easier.
     
      Your `uart.c` code should make it clear why you did what you did,
      and supporting reasons --- i.e., have page numbers and partial
      quotes for each thing you did.

   3. You should put your `uart.c` in `libpi/src` and
      update the `libpi/makefile`.   Remake your bootloader and put it
      on your sd card.

      You should then make sure that `make check` and `make checkoff`
      in lab 7 still works.

   4. Your fake pi implementation for `uart.c` gives the
      same hash as everyone else.  Note, there are multiple ways to do
      same thing, so maybe do the first one as a way to resolve ambiguity.

   5. Your software UART can reliably print and echo text between the pi
      and your laptop.

There are tons and tons of [EXTENSIONS](./EXTENSIONS.md)

-------------------------------------------------------------------------
#### The general goals

The main goal of this lab is to try to take the confusing prose and
extract the flow chart for how to enable the miniUART with the baud rate
set to 115200.  You're looking for sentences/rules:

  * What values you write to disable something you don't need
	(e.g., interrupts).

  * What values you have to write to enable things you need (miniUART).

  * How to get the gpio TX, RX pins to do what you need.

  * How to receive/send data.  The mini-UART has a fixed sized transmit
	buffer, so you'll need to figure out if there is room to transmit.
	Also, you'll have to detect when data is not there (for `uart_get8`).

  * In general, anything stating you have to do X before Y.

You don't have enough information to understand all the document.  This is
the normal state of affairs.  So you're going to start practicing how
to find what you need and interpolate the rest.

The main thing is to not get too worked up by not understanding something,
and slide forward to what you do get, and cut down the residue until
you have what you need.

-----------------------------------------------------------------------
### Part 1. implement `sw_put8` for a software UART.

<p align="center">
  <img src="images/debug-print.jpg" width="550" />
</p>

Checkoff summary:
   1. Implement the two `sw_uart_init_helper` and `sw_uart_put8` 
      routines in `sw-uart.c`.  

   2. Switch `libpi/Makefile` to use your code.

   3. `make checkoff` should pass: it just tests that you can 
     print `hello`.


One of the hardest things about writing the hardware UART driver is that
there is zero visibility into it when things go wrong.  

  - To put it in practical terms: Last year's class had about 1/4 of the
    students staying well past 1am with all sorts of nasty bugs.  Not much
    code, but the difficulty in debugging made it one of the 
    hardest labs in some ways.

So this year we won't do that.  To mitigate this problem, before starting
on the hardware we'll do a general purpose hack and write a software
UART implementation that uses GPIO to  "bit-bang" the UART protocol.
(Jonathan Kula did this in 2021's class as part of his final project.)
This will let you debug the hardware UART driver using print statements.

Two additional advantages of bit-banging:

 1. You will understand what the hardware is actually doing to implement
    UART.

    While the hardware folks in the class likely won't make this
    assumption-mistake it's easy as software people (e.g., me) to assume
    things such as: "well, the tty-usb device wants to talk to a 8n1-UART
    so I need to configure the pi hardware UART to do so."  This belief,
    of course, is completely false --- the 8n1 "protocol" is just a fancy
    word for setting pins high or low for some amount of time for output,
    and reading them similarly for input.  So, of course, we can do this
    just using GPIO.  (You can do the same for other protocols as well,
    such as I2C, that are built-in to the pi's hardware.)

 2. You'll also see that doing things using hardware is often
    much much harder than software.  In fact, given GPIO and a way to
    measure time, you can easily port your UART implementation to other
    hardware platforms in a few minutes.  Much easier than writing a
    hardware driver!  No datasheet, no erratta, etc.

As described in 
[UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter)
the protocol to transmit a byte `B` at a baud rate B is pretty simple.

  1. For a given baud rate, compute how many micro-seconds
     `T` you write each bit.  For example, for 115,200, this is:
     `(1000*1000)/115200 = 8.68`.  (NOTE: we will use cycles rather
     than micro-seconds since that is much easier to make accurate.
     The A+ runs at `700MHz` so that is 700 * 1000 * 1000 cycles per
     second or about `6076` cycles per bit.)

To transmit:
  1. write a 0 (start) for T.
  2. write each bit value in the given byte for T (starting at bit 0, 
     bit 1, ...).
  3. write a 1 (stop) for at-least T.


What to do:
  1. Implement `wait_ncycles_exact` in `1-sw-uart-put8/wait-routines.h`.
     And use `1-test-delay.c` to test it.  This test uses your routine
     to repeatedly delay by a fixed amount, measures the actual amount,
     and then prints out the error.  If you do things right you should
     get fairly low error.  E.g., I had about 45 cycles of error in 
     total for 10 samples:

```
        trial=0: measured = 601, expected 600, err = 1, tot err = 1
        trial=1: measured = 1200, expected 1200, err = 0, tot err = 1
        trial=2: measured = 1808, expected 1800, err = 8, tot err = 9
        trial=3: measured = 2407, expected 2400, err = 7, tot err = 16
        trial=4: measured = 3003, expected 3000, err = 3, tot err = 19
        trial=5: measured = 3608, expected 3600, err = 8, tot err = 27
        trial=6: measured = 4207, expected 4200, err = 7, tot err = 34
        trial=7: measured = 4803, expected 4800, err = 3, tot err = 37
        trial=8: measured = 5408, expected 5400, err = 8, tot err = 45
        trial=9: measured = 6000, expected 6000, err = 0, tot err = 45
    total error = 45 cycles, out of 6000 total cycles
```

  2. Using `wait_ncycles_exact` you should be able to implement
     a bit-banged UART routine to emit a 8-bit character
     `bitbang_uart_put8` in `2-test-sw-put8.c`.

     The most common problem people run into when doing so is that
     their timings are off, which will make printing always (or, worse,
     sometimes) fail.  Again, we want visibility.  To get it we 
     use the logging code above and check when writes are occuring.

     The file has code and an example (`log_example()`) on how to time
     `gpio_writes` easily to check that you are doing `gpio_write`'s
     at the right time and with the right values.  Do the same thing
     for your bit-banged code.  If your timing is reasonable, and the
     values are correct the code should just work.

     When running `bitbang_uart_put8(&sw, 'h')` I get:

```
    sw uart: cycles per bit=6076 for baud=115200
    done with sw-uart example: time log has 10 entries:
        0: v=0: [value of first reading]
        1: v=0: measured = 6137, expected 6076, err = 61, tot err = 61
        2: v=0: measured = 12224, expected 12152, err = 72, tot err = 133
        3: v=0: measured = 18275, expected 18228, err = 47, tot err = 180
        4: v=1: measured = 24315, expected 24304, err = 11, tot err = 191
        5: v=0: measured = 30424, expected 30380, err = 44, tot err = 235
        6: v=1: measured = 36503, expected 36456, err = 47, tot err = 282
        7: v=1: measured = 42585, expected 42532, err = 53, tot err = 335
        8: v=0: measured = 48661, expected 48608, err = 53, tot err = 388
        9: v=1: measured = 54795, expected 54684, err = 111, tot err = 499
    total error = 499 cycles, out of 60760 total cycles
```
     If you get something similar your code should just work.

  3. Now implement the two routines in `sw-uart.c`.  You'll need
     to setup the transmit and receive GPIO pins (output and input
     respectively) and drop in your bitbangged code.  When you
     do this, `3-hello.c` should just work.  If you want to 
     play around you can uncomment the staff binary.

  4. Switch your `libpi/Makefile` to use `sw-uart.c` by
     adding:

        # libpi/Makefile
        SRC += ../labs/8-uart/1-sw-uart-put8/sw-uart.c

      To the start commenting `sw-uart.c` out this directory's `Makefile`.
      `make check` should still work.

NOTE: debugging:

  - To use your softare UART code in the next step, you would just include
    `sw-uart.h` into (`uart.c`), create a software UART structure using
    `sw_uart_init` and then use the print routines.

  - COMMON MISTAKE: running the software uart will destroy some of
    the hardware UART's state (the GPIO pins), so put these last in setup
    or make sure you remove all uses of the software uart when you want
    to run with the hardware.

-----------------------------------------------------------------------
### Part 2. implement a UART device deriver:

Our general development style will be to write a new piece of
functionality in a private lab directory where it won't mess with anything
else, test or (better) equivalence check it, and then, migrate it into
your main `libpi` library so it can be used by subsequent programs.

Concretely, you will implement the routines in `8-lab/2-uart/uart.c`.
The main tricky ones:

  1. `void uart_init(void)`: called to setup the miniUART.  It should
     set the baud rate to `115,200` and leave the miniUART in its default
     `8n1` configuration.  Before starting, it should explicitly disable
     the UART in case it was already running (since we bootloader,
     it will be).

  2. `int uart_get8(void)`: blocks until it can read a byte from
     the mini-UART, and returns the byte as a signed `int` (for sort-of
     consistency with `getc`).

  3. `void uart_put8(unsigned c)`: puts the byte `c` onto the UART transmit
     queue.  If necessary, it blocks until there is space.

General approach for `uart_init`:
  1. You need to turn on the UART in AUX.  Make sure you
     read-modify-write --- don't kill the SPIm enables.
  2. Immediately disable tx/rx (you don't want to send garbage).
  3. Figure out which registers you can ignore (e.g., IO, p 11).
     Many devices have many registers you can skip.
  4. Find and clear all parts of its state (e.g., FIFO queues) since we
     are not absolutely positive they do not hold garbage.  Disable
     interrupts.
  5. Configure: 115200 Baud, 8 bits, 1 start bit, 1 stop bit.  No flow
     control.
  6. Enable tx/rx.  It should be working!

If you run `make` in `4-uart/2-uart` it will build:
  - A simple `hello` you can use to test. I'd suggest shipping it over with your
    bootloader.  

A possibly-nasty issue: 

  1. If you test using the bootloader (recommended!), that code obviously
    already initializes the UART.  As a result, your code might appear
    to work when it does not.

  2. To get around this issue, when everything seems to work, change
     the `Makefile` to use `output-test-1-hello-disable.c`.
     This will repeatedly disable and enable the uart --- not a perfect
     test, but a bit more rigorous.
 
  3. Once your code works, make swap out the `uart.c` in the `libpi/Makefile`
     to use yours:  remove it from the current
     `2-uart/Makefile`, recompile.

        # libpi/Makefile
        SRC += ../labs/8-uart/2-uart/uart.c

  4. Make sure `make check` for the tests still work.

-----------------------------------------------------------------------
### Part 3. re-install your bootloader with the new uart code.

Now change your bootloader to use the new `uart.c`:

  1. Add your `uart.c` to your `libpi/Makefile`.

        SRC += ../labs/8-uart/2-uart/uart.c

  2. Print "UART" alongside your name in your `get-code.h`
  3. Copy it as `kernel.img` to your SD card.
  4. Nothing you changed should have made any difference in behavior.
     So: Make sure that `2-uart` tests still pass after replacing the
     bootloader.
     Then do `make checkoff` in the previous lab `7-bootloader/checkoff`
     to make sure they still work, too.
  
-----------------------------------------------------------------------
##### Part 3. `3-fake-pi` - Not required for this year's checkoff


For the output I get:
```
        % cd 3-fake-pi
        % make
        % cd tests-uart
        % make
        % make emit
        % make checkoff

        individual checksums = 
        852325631 5396 0-uart-getc.out
        1030750329 808 0-uart-init.out
        1334889765 1577 1-hello.out
        3030943895 229 0-uart-putc.out
        checksum(checksum) = 
        2291128381 121

```

You might get different output if you use different registers.  Check with
your partner!


My `3-fake-pi/tests-uart/0-uart-init.out`:

```
    TRACE: out file for <0-uart-init>
    TRACE:0: calling pi code
    TRACE:1: dev_barrier
    TRACE:2: GET32(0x20200004) = 0x643c9869
    TRACE:3: PUT32(0x20200004) = 0x643ca869
    TRACE:4: GET32(0x20200004) = 0x643ca869
    TRACE:5: PUT32(0x20200004) = 0x643d2869
    TRACE:6: dev_barrier
    TRACE:7: GET32(0x20215004) = 0x0
    TRACE:8: PUT32(0x20215004) = 0x1
    TRACE:9: dev_barrier
    TRACE:10: UART: PUT32(0x20215060) = 0x0
    TRACE:11: UART: turning UART off PUT32(20215060)=0
    TRACE:12: UART: PUT32(0x20215044) = 0x0
    TRACE:13: UART: PUT32(0x20215048) = 0x6
    TRACE:14: UART: PUT32(0x2021504c) = 0x3
    TRACE:15: UART: PUT32(0x20215050) = 0x0
    TRACE:16: UART: PUT32(0x20215060) = 0x3
    TRACE:17: UART: PUT32(0x20215068) = 0x10e
    TRACE:18: UART: turned UART on PUT32(20215060)=3
    TRACE:19: dev_barrier
    TRACE: pi exited cleanly: 20 calls to random
```

Some notes:

  1.  If you look in the `3-fake-pi/Makefile` you see it's getting your
      `gpio.c` and `uart.c` and pulling them into fake-pi.

  2. If you look in fakepi you see there is some logic to buffer
     the UART writes while the tx and rx is disabled (the control
     register). When the control is turned on, all buffered writes 
     get written.

  3. `cd` into `3-fake-pi/tests-uart` and run the test for
     `0-uart-init.c`.  You may have addresses written in order (there
     are multiple legal orders). If you don't match ours,  check with
     someone else and have some reason that the reorder you do is ok.

     You'll have to compare your `.out` files with other people.  You
     probably want to run these by hand one at a time.

<p align="center">
  <img src="images/rpi-cables.png" width="450" />
</p>

---------------------------------------------------------------------
## Extension: `sw_uart_get8`
Adding input is good.  Two issues:
  1. The GPIO pins (obvious) have no buffering, so if you are reading
     from the RX pin when the input arrives it will disappear.

  2. To minimize problems with the edges of the transitions being off
     I'd have your code read until you see a start bit, delay `T/2` and then
     start sampling the data bits so that you are right in the center of 
     the bit transmission.
