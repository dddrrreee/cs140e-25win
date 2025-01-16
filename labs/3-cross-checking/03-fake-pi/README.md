## Simple examples of how to run pi programs on Unix

Key ideas:
  - you can take code written to run on an r/pi on bare metal and 
    redefine primitives so it can run on your laptop.

  - by running on your laptop you get memory protection, debugging,
    other tools.

  - even a small amount of code lets you run pi programs on Unix and get
    get the same result.

Rough organization:
 - `Makefile` defines different options (`0-libpi-fake`, `1-libpi-fake`,
   `2-libpi-fake`) that get increasingly fancy.

   This main makefile calls two sub-Makefiles.

 - have one makefile to compile for the pi (`Makefile.pi`)

        % make pi
        ...
        bootloader: Done.
        listening on ttyusb=</dev/ttyUSB0>
        hello world
        DONE!!!
        
 - have one makefile to compile on unix (`Makefile.fake-pi`)

        % make fake-pi
        running our fake pi program
        ./hello.fake
        hello world
        DONE!!!

-----------------------------------------------------------------------
### `0-libpi-fake`

Simplest possible "fake pi"  where we just redefine printk and
clean_reboot in a private rpi.h  (`0-libpi-fake/rpi.h`) and include it
into a pi program.

This is the most basic version.  
It only uses the routines in:
 1. `0-libpi-fake/libpi-fake.c` (for the `notmain` caller)
 2. `0-libpi-fake/rpi.h` header file, which defines `printk` and
`clean_reboot`.


Even though it only defines `printk` and `clean_reboot` it has enough
to run the two pi programs on Unix:
  - `hello.c`: print hello world. 
  - `null-ptr.c`: prints messages and dereferences a null pointer.
     This dereference won't crash on the pi (no memory protection yet).
     But will crash on Unix.  This shows why it can be useful to run
     code on a fake environment.

Running `make` will compile and run both programs on unix:

        % make
        ... compilation ...
        ------------------------------------------------------ 
                running our fake pi program: <hello.fake>         
        ./hello.fake
        hello world
        DONE!!!
        ------------------------------------------------------ 
                running our fake pi program: <null-ptr.fake>         
        ./null-ptr.fake
        about to write to null [(nil)]
        will crash on Unix (good!  detect bug) will succeed on pi (bad)


As promised, if you run on the pi you get the same for `hello.c` and a
different for `nullptr.c`:

    % make pi
    bootloader: pi exited.  cleaning up
    pi-install  ./hello.bin
    ...
    listening on ttyusb=</dev/ttyUSB0>
    hello world
    DONE!!!

    pi-install  ./null-ptr.bin
    ...
    about to write to null [0x0]
    will crash on Unix (good!  detect bug) will succeed on pi (bad)
    null write succeeded??   result = 1
    DONE!!!


-----------------------------------------------------------------------
### `1-libpi-fake`

This version starts getting more real.  The big change is that we use
the raw `rpi.h` header file from the libpi directory at the top of the
class repo (`libpi/include/rpi.h`).

Because we use `libpi`'s header we move the versions of `printk` and
`clean_reboot` into `libpi-fake.c`.

We run this version by changing `03-fake-pi/Makefile` so that the last
assignment to `FAKE_VERSION` is:

    # 03-fake-pi/Makefile
    FAKE_VERSION = 1-libpi-fake


If you run `make` you see we get the same result as before.

Because we use the raw `rpi.h` we have more macros, such as `demand` and
`assert` and `debug` (these are in `libpi/libc/demand.h`).  This
means we can run additional programs:
  -  `test-assert.c`: shows how to use `assert` to panic if a condition
     is violated.
  - `test-demand.c`: similar, with a more informative error message.


Add these programs in `03-fake-pi/Makefile`:

    # can uncomment these for 1-libpi-fake or 2-libpi-fake
    PI_PROGS += test-demand.c
    PI_PROGS += test-assert.c


Now when you run:

    % make
    ------------------------------------------------------ 
        running our fake pi program: <test-demand.fake>         
    ./test-demand.fake
    test-demand.c:notmain:4:DEBUG:demand should fail
    test-demand.c:notmain:6:DEBUG:ERROR: Demand `x < 4` failed:testing demand: this should fail
    DONE!!!
    ------------------------------------------------------ 
        running our fake pi program: <test-assert.fake>         
    ./test-assert.fake
    test-assert.c:notmain:4:DEBUG:assert should fail:
    test-assert.c:notmain:6:DEBUG:ERROR: Assertion `x < 4` failed.
    DONE!!!

    ... deleted ...


If you run on the pi (`make pi`) you'll get the same.

Great! More code runs and if you run it on the pi you should get the same
results.

-----------------------------------------------------------------------
### `2-libpi-fake`

Now we take a big step and start including much lower level files from
libpi.  Again, the interesting thing here is that these files were written
to run on the pi and yet if we implement the right low level interfaces
we can fake them out and run them on unix without any modification.

If you look in `2-libpi-fake/Makefile` you can see that we pull in
the entire file for `libpi/libc/printk.c`
`libpi/libc/putk.c`
`libpi/libc/putchar.c`
and `libpi/libc/clean-reboot.c`:

    # 2-libpi-fake/Makefile : pull in more files.
    SRC  = $(LPP)/libc/printk.c
    SRC  += $(LPP)/libc/putk.c
    SRC  += $(LPP)/libc/putchar.c
    SRC  += $(LPP)/staff-src/clean-reboot.c

If you look in these files you can see they are pretty busy doing
pi stuff.  The fact we can use them without alteration in a vastly
different context is worth thinking about off and on across the quarter.

You can see that `2-libpi-fake/libpi-fake.c` defines just a few routines:
  - `uart_putc` : on the pi, print 8 bytes to the uart.  On Unix, we just
     print to `stdout`.
  - `uart_flush_tx`: flush UART output.  On unix flush `stdout`.
  - `delay_ms`: delay for a given number of millseceonds.  On Unix do nothing.
  - `rpi_reboot`: reboot the pi.  On unix just `exit`.

If we do `make` or `make pi` we get the same results as before.

There's a lot more than can be done to do real emulation/simulation.
This is just a hello world level view.  With that said, it's enough
to do some interesting stuff in today's lab.
