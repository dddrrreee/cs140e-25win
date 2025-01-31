## Lab: write your own UART implementation.



------------------------------------------------
#### Clarification errata

Note:
  - the staff uses `lrc` and you are most likely using `stat`
    so the checksums for routines other than `uart_init` 
    likely won't match.  make sure you do match with another
    group.

    then: please post your checksum to ed so we can all agree.
  - if your fake-pi is not terminating do a pull.  lmk if 
    that doesn't fix it.
  - The final part 3 sw-uart is pushed!
  - do the GPIO pins first (top of page 10) "GPIO pins should
    be setup first before enabling the UART".  Then do 
    the AUX enable.  Then set the uart.
  - figure out all registers you can ignore and set them to
    the default values.
  - for cross checking, set the UART registers in ascending
    order when possible.
------------------------------------------------


<p align="center">
  <img src="images/uart-meme.jpg" width="450" />
</p>

Make sure you've read the 
[miniUART cheat sheet](../../notes/devices/miniUART.md).


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
   1. `1-uart`: put your `uart.c` in `libpi/src` and
      update the `libpi/Makefile`.   `make checkoff` in `1-uart` passes.
      Remake your bootloader and put it on your sd card: the 
      tests from lab 6 should pass.

      Your `uart.c` code should make it clear why you did what you did,
      and supporting reasons --- i.e., have page numbers and partial
      quotes for each thing you did.

   2. Your fake pi implementation for `uart.c` gives the
      same hash as everyone else.  Note, there are multiple ways to do
      same thing, so always set the the lower addresses before the higher.

   3. Your software UART can reliably print and echo text between the pi
      and your laptop.

There are tons and tons of [EXTENSIONS](./EXTENSIONS.md)

-------------------------------------------------------------------------
### The general goals

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
### Part 1. implement a UART device deriver `1-uart/uart.c`

#### Before you start

Check that everything works.

  1. Check that `make check` in `1-uart` works.

            % cd 1-uart
            % make check

  2. If that works, uncomment the `Makefile` line so that it uses
     the local `uart.c`

            COMMON_SRC += uart.c

  3. Check that `make run` gives "must implement" errors.

#### Write the code in `uart.c`

Our general development style will be to write a new piece of
functionality in a private lab directory where it won't mess with anything
else, test or (better) equivalence check it, and then, migrate it into
your main `libpi` library so it can be used by subsequent programs.

Concretely, you will implement the routines in `1-uart/uart.c`.
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

General:
   - From broadcom: if you are writing to different 
     devices you MUST use a `dev_barrier()`.
   - Its not always clear when X and Y are different
     devices.
   - We assume we don't know what the client code
     was doing, so as with the interrupt handler and unlike GPIO code:
     each time you come into or leave the uart code, do a device barrier.
   - Pay attenton for errata!   There are some serious
     ones here.  If you have a week free you'd learn 
     alot figuring out what these are (esp hard given
     the lack of printing) but you'd learn alot, and
     definitely have new-found respect to the pioneers
     that worked out the BCM eratta.

A possibly-nasty issue: 

  1. Given that we load all the programs using the bootloader,
     that code obviously already initializes the UART.  As a result,
     your code might appear to work when it does not.

  2. To get around this issue, when everything seems to work, change
     the `Makefile` to use `output-test-1-hello-disable.c`.
     This will repeatedly disable and enable the uart --- not a perfect
     test, but a bit more rigorous.

#### Hack to make things easier

Historically a problem with writing UART code for this class (and for
human history) is that when things go wrong you can't print since doing
so uses uart.  Thus, debugging is very old school circa 1950s, which
modern brains aren't built for out of the box.   You have two options:

  1. Think hard.  we recommend this.
  2. Use the included bit-banging software UART routine
     to print.   This makes things much easier.
     but if you do make sure you delete it at the 
     end, otherwise your GPIO will be in a bad state.

In either case, in the final part of the lab you'll implement bit-banged
UART yourself.
  
-----------------------------------------------------------------------
### Part 2. cross check your UART `2-fake-pi` 

UART bugs are incredibly nasty.   Unfortunately, a broken UART driver
can easily work on simple tests but fail on more complicated ones.
We'll use the cross-checking idea from lab 3 to compare the `PUT32` and
`GET32` that your code against everyone else.  Our mantra: If everyone
matches and one person was right, then everyone was right.

To do this, we
extend the `fake-pi.c` code from `3-cross-checking` to handle 
UART and AUX writes.  If you look in `2-fake-pi/Makefile` you'll
see that we link in your `libpi/src/gpio.c` and `../1-uart/uart.c`.
There are simple tests for the UART in `2-fake-pi/tests-uart`

To check your output:
```
        % cd 2-fake-pi/tests-uart
        % make emit
        % make checksum

        individual checksums = 
        852325631 5396 0-uart-getc.out
        1030750329 808 0-uart-init.out
        1334889765 1577 1-hello.out
        3030943895 229 0-uart-putc.out
        checksum(checksum) = 
        2291128381 121

```

NOTE:
  - You might get different output if you do things differently than
    we do.  If so, you should have agreement in a group that your
    output makes sense (and matches theirs!)
  - There are many cases where you could write one UART address
    before another.  Since we do dumb exact matching of traces,  this
    will lead to mismatches.  So you should always write the lower
    address first.
  - Once you have done a `make emit` to generate .out files you can 
    do a `make check` to make sure that your behavior doesn't differ
    when you make changes.


My `2-fake-pi/tests-uart/0-uart-init.out`:

```
TRACE:0: calling pi code
TRACE:1: dev_barrier [0]
TRACE:2: GET32(0x20200004) = 0x643c9869
TRACE:3: PUT32(0x20200004) = 0x643ca869
TRACE:4: GET32(0x20200004) = 0x643ca869
TRACE:5: PUT32(0x20200004) = 0x643d2869
TRACE:6: dev_barrier [1]
TRACE:7: GET32(0x20215004) = 0x0
TRACE:8: PUT32(0x20215004) = 0x1
TRACE:9: dev_barrier [2]
TRACE:10: UART: PUT32(0x20215060) = 0x0 [is on! off=0]
TRACE:11: UART: turning UART off PUT32(20215060)=0
TRACE:12: UART: PUT32(0x20215044) = 0x0 [buffered]
TRACE:13: UART: PUT32(0x20215048) = 0x6 [buffered]
TRACE:14: UART: PUT32(0x2021504c) = 0x3 [buffered]
TRACE:15: UART: PUT32(0x20215050) = 0x0 [buffered]
TRACE:16: UART: PUT32(0x20215060) = 0x3 [buffered]
TRACE:17: UART: PUT32(0x20215068) = 0x10e [buffered]
TRACE:18: UART: turned UART on PUT32(20215060)=3
TRACE:19: dev_barrier [3]
```

Some notes:

  1. If you look in `fakepi.c` you see there is some logic to buffer
     the UART writes while the TX and RX is disabled (the control
     register). When the control is turned on, all buffered writes 
     get written.

  2. `cd` into `2-fake-pi/tests-uart` and run the test for
     `0-uart-init.c`.  You may have addresses written in order (there
     are multiple legal orders). If you don't match ours,  check with
     someone else and have some reason that the reorder you do is ok.

     You'll have to compare your `.out` files with other people.  You
     probably want to run these by hand one at a time.

<p align="center">
  <img src="images/rpi-cables.png" width="450" />
</p>


-----------------------------------------------------------------------
#### Part 3: install your `uart.c`!

Now that your UART works compared to everyone else, install in
libpi and use it the in the bootloader.


Swap out the `uart.c` in the `libpi/Makefile` to use yours:  

  1. Copy `1-uart/uart.c` to `libpi/src/uart.c` and change the 
     the libpi `Makefile`


            # add this to libpi/Makefile
            SRC += src/uart.c

            # comment this out from libpi/Makefile
            # STAFF_OBJS  +=  ./staff-objs/uart.o

  2. `make clean`, recompile.
  3. Remove `uart.c` from the `1-uart/Makefile`:


            # 1-uart/Makefile: comment this out!
            # COMMON_SRC += uart.c

  4. Make sure `make check` for the `1-uart` tests still work.


Re-install your bootloader with the new uart code:

  1. Add "UART" to the print of your name in your `get-code.h`.
  2. Recompile your bootloader and put on your SD card.
  3. Make sure the `6-bootloader/checkoff` tests still work.


-----------------------------------------------------------------------
### Part 4. implement `sw_put8` for a software UART.

<p align="center">
  <img src="images/debug-print.jpg" width="550" />
</p>


Checkoff summary:
   1. Implement the two `sw_uart_init_helper` and `sw_uart_put8` 
      routines in `3-sw-uart-put8/sw-uart.c`.  

   2. `make checkoff` should pass: it just tests that you can 
      print `hello`.

   3. Switch `libpi/Makefile` to use your code.

One of the hardest things about writing the hardware UART driver is that
there is zero visibility into it when things go wrong.  

  - To put it in practical terms: Last year's class had about 1/4 of the
    students staying well past 1am with all sorts of nasty bugs.  Not much
    code, but the difficulty in debugging made it one of the 
    hardest labs in some ways.

So this year we didn't do that and used a software version that uses
GPIO to  "bit-bang" the UART protocol so you could debug the hardware
UART driver using print statements.  (Jonathan Kula did this in 2021's
class as part of his final project.)  You'll now build this.

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

To initialize:
  1. Setup the given GPIO pins as input (RX) and output (TX).
  2. Make sure the TX line has its correct default value.

To transmit:
  1. write a 0 (start) for T.
  2. write each bit value in the given byte for T (starting at bit 0, 
     bit 1, ...).
  3. write a 1 (stop) for at-least T.

---------------------------------------------------------------------
### Extensions.

Other extensions are in [EXTENSIONS](EXTENSIONS.md)

#### Trace `uart_init()`

Change your tracing code from lab 3 to be able to trace UART.  You'll have
to buffer the `PUT32` and `GET32` so that you can dump them later,
otherwise you'll be trying to print when UART is being initialized,
which won't work.

#### Change the baud

As we make more complicated programs, the bootloading overhead will add
up (you've seen this in past checkoffs).  So an interesting puzzle is
how fast you can make the baud rate for your bootloader.  Note you'll
have to change the Unix side too and, at some point, the unix won't
be able to keep up.

#### Add input: `sw_uart_get8`

Adding input is good.  Two issues:
  1. The GPIO pins (obvious) have no buffering, so if you are reading
     from the RX pin when the input arrives it will disappear.

  2. To minimize problems with the edges of the transitions being off
     I'd have your code read until you see a start bit, delay `T/2` and then
     start sampling the data bits so that you are right in the center of 
     the bit transmission.
