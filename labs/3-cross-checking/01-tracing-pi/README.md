### How to override routines in .o files at link-time.

This shows an example of how to use the GNU linker `ld` to interpose
on all calls to a given routine (or set of routines) by using the
`--wrap` flag.  In the example code we override the definition of `foo`
in `trace-foo.c` as follows:

  1. `Makefile` uses the `--wrap=foo` flag to override `foo`.
  2. The linker will replace all calls to `foo` with calls to `__wrap_foo`.  
  3. The linker will also rename the definition of `foo` as `__real_foo`.
  4. `trace-foo.c` defines a `__wrap_foo` that calls `__real_foo`.

If you run this on your pi it should print:

        % pi-install trace-foo.bin 
        .. a bunch of stuff ...
        about to call foo(1,2)
        real foo: have arguments (1,2)
        returned  3
        DONE!!!
The code is small, so look at it!

### For lab

For lab: We will use this linker trick in lab to interpose on `GET32`,
`get32`, `PUT32` and `put32` so that you can trace what calls your
`gpio.c` makes without modifying the source.

For example, to override `GET32` we do the same steps as above:

  1. When linking the a program, add a `--wrap=GET32` argument to 
     the linker command.
  2. The linker will replace all calls to `GET32` with calls to `__wrap_GET32`.  
  3. The linker will also rename the definition of `GET32` as `__real_GET32`.

Then, in order to trace all `GET32` operations you would implement a
`__wrap_GET32(unsigned addr)` routine which calls `__real_GET32(addr)`
to get the value for `addr` and prints both.

You would override `PUT32` similarly.
