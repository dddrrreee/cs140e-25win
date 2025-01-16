### Prelab for fake-pi

Reading:
  1. The pullup/pulldown discussion in the broadcom document (see more
     discussion below).  You're going to implement this.
  2. Read [03-fake-pi/README.md](03-fake-pi/README.md) to see simple
     examples of emulating pi code on Unix.
  3. Read the [1-fake-pi/README.md](1-fake-pi/README.md) a few
     times and look through the code.  Understanding how to run r/pi
     code unaltered on a fake pi Unix emulator is a crucial concept that
     we'll use throughout the quarter.
  4. Read the lab for more discussion and to see where we are going.

What to do:

   1. `01-tracing-pi`: run and look through this code.  it's a
      simple example of how to override routines using the linker.
      You will use this to trace the PUT/GET calls when run on the
      actual pi hardware and do the same comparison to everyone else,
      and compare to when you replace our staff `gpio.o` with your own.

   2. `02-tracing-linux` if you're on linux you can also run similar code
      for step 2.  AFAIK MacOS's linker  does not support this option.

   3. `03-fake-pi`: this shows increasingly fancy examples of taking
      r/pi code and tricking it into running on your laptop (i.e.g.,
      not the pi hardare) by making fake versions of different r/pi
      library routines.

      It is setup to run both on the pi and your laptop.   You should
      look through the subdirectories (`0-libpi-fake` `1-libpi-fake`
      `2-libpi-fake`) and try running them by changing the
      `03-fake-pi/Makefile` to use the different version.

   4. `1-fake-pi` : a more full fledged but still fairly
      trivial user-level emulator for your pi code.  You will use this
      approach to cross-check your implementation against everyone else's
      to ensure everyone is equivalant.  Thus, if one person is correct,
      everyone is correct.

To pop up a few levels, the raw tracing of PUT/GET enables some
interesting tricks:

  1. Check fidelity of your fake-pi emulator:  You can store the PUT/GET
     calls emitted from a raw run to a file (a log) and then change the
     fake pi so it optionally reads from this log: when you replay them
     the result should be identical.  (Including when you log the PUT/GET
     done during the replay, emit that to a log, replay that, etc.)

  2. Partial evaluation: You can also write a program that reads this
     log and emits a pi program that executes just these exact calls ---
     you can use this trick to replace the original program them with this
     (often much simpler) low-level sequence of PUTs and GETs.

------------------------------------------------------------------
### Configuring r/pi pullup/pulldown resistors.

As part of the lab you'll implement `gpio_set_pullup` and
`gpio_set_pulldown`

By default, an input pin will be "floating" if it's disconnected. This
means it'll return 0 or 1 at random. When we want a known, fixed default
value, we can enable what are called "pullup" or "pulldown" resistors,
which bring the default value to 1 or 0 respectively.

These are described, with much ambiguity, in the Broadcom document on
page 101 (using my pdf reader page numbering). Some issues:

1. They say to use clock delays, but do not way which clock (the pi clock?
   The GPU clock? Some other clock?)

2. A straight-forward reading of steps (5) and (6) imply you have
   to write to the `GPPUD` to and `GPPUDCLK` after setup to signal
   you are done setting up. Two problems: (1) write what value? (2)
   the only values you could write to `GPPUD`, will either disable the
   pull-up/pull-down or either repeat what you did, or flip it.

In other domains, you don't use other people's implementation to make
legal decisions about what acts are correct, but when dealing with
devices, we may not have a choice (though, in this case: what could we
do in terms of measurements?).

Two places you can often look for the pi:

1. Linux source. On one hand: the code may be battle-tested, or
   written with back-channel knowledge. On the other hand:
   it will have lots of extra Linux puke everywhere, and linux is far from not-buggy.

2. `dwelch76` code, which tends to be simple, but does things (such as
   eliding memory barriers) that the documents explicitly say are wrong
   (and has burned me in the past).

For delays:
[Linux]
(https://elixir.bootlin.com/linux/v4.8/source/drivers/pinctrl/bcm/pinctrl-bcm2835.c#L898) uses 150 usec. [dwelch76]
(https://github.com/dwelch67/raspberrypi/blob/master/uart01/uart01.c)
uses something that is 2-3x 150 pi system clock cycles. The
the general view is that we are simply giving the hardware "enough" time to settling
into a new state rather than meeting some kind of deadline and, as a result,
that too-much is much better than too-little, so I'd go with 150usec.

For what to write: from looking at both Linux and dwelch67 it _seems_
that after steps (1)-(4) at set up,
you then do step (6), disabling the clock, but do not do step (5) since its simply
hygenic.

Since we are setting "sticky" state in the hardware and mistakes lead to
non-determinant results, this is one case where I think it makes sense
to be pedantic with memory barriers: do them at the start and end of the
routine. (NOTE, strictly speaking: if we are using the timer peripheral
I think we need to use them there too, though I did not. This is a place
where we should perhaps switch to using the cycle counter.)

If you get confused, [this
page](http://what-when-how.com/Tutorial/topic-334jc9v/Raspberry-Pi-Hardware-Reference-126.html)
gives an easier-to-follow example than the broadcom.
