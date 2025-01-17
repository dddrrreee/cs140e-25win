### Lab: automatically cross-check your GPIO code against everyone else's.

----------------------------------------------------------
***Errata***:
***Errata***:
***Errata***:
***Errata***:

  - `2-trace`: this `Makefile` has issues, so you may need
    to run `make` in `2-trace` and then inside `2-trace/test`
    this won't be true for the rest of the quarter.  I forgot
    to fix a problem from last year.
    
  - if your tracing just hangs this is b/c there is an
    infinite loop from printing: `printk` will call the
    UART driver, which will then call `put32` and `get32`
    which you will then call `printk` for etc.  this is
    a pretty common issue when writing low level monitor
    code. soln: don't trace when you've called `printk`.

  - don't check for 47 in `gpio_read` do:

            int gpio_read(unsigned pin) {
                if(pin >= 32)
                    return -1;
                ...
                return DEV_VAL32(x);
            }

  - Please make sure your class path definition ends with
    a `/`.  For example:

        setenv CS140E_2025_PATH /home/engler/class/cs140e-25win/

    Versus

        setenv CS140E_2025_PATH /home/engler/class/cs140e-25win


----------------------------------------------------------

A goal of this course is that you will write every single line of
(interesting) low level code you use.  A good result of this approach
is that you will understand everything and, unlike most embedded or
operating systems courses, there will not be a lot of opaque, magical
code that you have no insight into, other than sense of unease that it
does important stuff beyond your ken.

An obvious potential result of this strategy is that since you are writing
all code your system depends on, a single bug can make the rest of the
quarter miserable.

Today's lab uses simple implementations of several powerful ideas to
check if your `gpio.c` code from last lab has bugs.

After completing the lab you will be able to check that your `gpio.c`
code is effectively equivalent to everyone else's in the class by
tracing all reads and writes it does and comparing them to everyone
else's implementation.  If even one person gets it right, then showing
equivalence means you got it right too.  And, nicely, automatically
detect if any subsequent modifications you do break the code.

The basic intuition: it's very hard to check if two different Turing
machines are equivalant by comparing their logic.  On the other hand
it's trivial compare their tapes after they run: same end tape = same
computed result.


You'll do two things:
  1. Run your code in a simple fake simulator that you'll modify
     so that you can trace the memory reads and writes it does to 
     GPIO memory.  

  2. Using the "wrap" linker trick to trace the code on the raw 
     hardware.  

----------------------------------------------------------------------
#### tl;dr: checking code

Checking your code:

   1. Your checksums for the tests in `1-fake-pi/tests` match your
      partner's (and ours).

      Note this includes implementing `gpio_set_function` and check that it 
      gives the same checksum. 

      The easiest checkoff method is edit 
      `1-fake-pi/tests/Makefile` and uncomment the line:

            # 6. then do everything.
            TEST_SRC := $(wildcard ./[0-5]-*.c) ... [more files]

      and run in the same directory (`1-fake-pi/tests`):

            % make checkoff

      This reduces all the output to a single number you can compare
      at a glance with everyone else.  Note: you'll have to work with
      everyone to figure out who is wrong when there is a difference.

      This is equivalant to  manually running:

            # compute the cksum of your cksums.
            1-fake-pi/tests % ls ./*.out | sort | xargs cksum | cksum

   2. `2-trace`: You get the same checksum for all the `.out` files
      produced by the tests in `2-trace/tests` --- note, there can be a
      differences in the intial values for GPIO pins when comparing pi
      zeroes to pi A+s.

   3. When you put in your `gpio.o` into `libpi` you get the same output
      for (2).

   4. ***NOTE: we're modifying this description.***
      After everything works, add gpio pullup and gpio pulldown and check
      with everyone else.  You'll have to add timing routines!

------------------------------------------------------------------------
#### Checksums.

If you want to use our checksums:

		% cd 1-fake-pi/test
		% make checkoff

        cksums of all files in (CHKOFF_OUT):
        950806854 132 ./0-gpio-read-17.out
        2950566669 132 ./0-gpio-read-20.out
        950806854 132 ./0-gpio-read-21.out
        2950566669 132 ./0-gpio-read-7.out
        197440487 149 ./0-gpio-set-input-17.out
        2488920004 149 ./0-gpio-set-input-20.out
        4071544067 149 ./0-gpio-set-input-21.out
        233099041 149 ./0-gpio-set-input-7.out
        2970604951 143 ./0-gpio-write-17.out
        3157623788 145 ./0-gpio-write-21.out
        2647062284 2139 ./1-gpio-read.out
        2830168373 5379 ./1-gpio-set-input.out
        107627159 107 ./1-gpio-set-off.out
        710888840 107 ./1-gpio-set-on.out
        53206415 149 ./1-gpio-set-output.out
        2984310116 2460 ./1-gpio-write.out
        1190846849 2679 ./2-set-input-N.out
        1585249846 1262 ./2-set-off-N.out
        2356184348 1262 ./2-set-on-N.out
        1674533470 2685 ./2-set-output.out
        3263639091 634 ./5-gpio-n-set-func.out
        688829614 149 ./5-gpio-set-func.out
        2119988811 5381 ./5-set-function-N.out
        815968718 634 ./5-set-pin-func.out
        2608020824 149 ./act-set-output.out
        3166146642 141 ./act-write.out
        3264505994 1602 ./prog-1-blink.out
        2701669809 2464 ./prog-2-blink.out
        1444919731 4598 ./prog-3-input.out
        total files = 29
        USE THIS VALUE FOR CHECKOFF:
        525118589 1049
        
There's a bunch of extensions.

----------------------------------------------------------------------
#### How to turn in after your local checks pass.

On your local machine:
  - in directory: `1-fake-pi/test` running `make checkoff` should give
    the checksum: `525118589`.

To checkoff:
- First make sure you have manually made sure your code works on your computer.
- Reclone the autograder from here: http://github.com/dghosef/140e-dumb-autograder. Or pull and fix the conflict
- Modify the repo, sunet, and lab variables in `sender.py`. The lab one should be set to 'lab3'
- Run `sender.py`. The output will be pushed to the checkoff directory of http://github.com/dghosef/140e-dumb-autograder
- Make sure the checksums make sense
 
- Let Joe know if something isn't working

------------------------------------------------------------------------
#### Background: Concepts you will learn.

Today's lab has relatively little code.  However, this code gives
usefully working examples of several powerful ideas:

  1. You will see how to build a trivial, but useful fake-pi "simulator"
     (`code/fake-pi.c`) that can run your r/pi code on your Unix laptop
     instead of the r/pi hardware, *without* requiring any modification
     other than re-compilation.  
    
     One nice result of running on your laptop is that it makes it
     much easier to debug bare-metal r/pi code because you have memory
     protection (so dereferencing a null or illegal pointer will cause a
     crash) and a debugger (so you can find such bugs easily).  A second
     nice feature is the next:

  2. You will also see how to use the fake-pi code to do read-write
     equivalance checking, a simple, little known trick that lets you
     easily check your code behaves identically to other people's,
     even if their code looks very different.

     The end result: with a couple hours of work, you will be
     very surprised if your code has bugs. You will also be able to
     immediately detect many cases where a later modification causes
     different behavior (e.g., if you speed the code up, clean it up, use
     different compiler options, etc.)   We will use this trick constantly
     thoughout the quarter.  (And I think all embedded software should
     be built using some variation of it.)

  3. You will also see the power of using cross-checking to detect
     incorrect code.  Often checking correctness even very-partially
     takes more code than the actual code being checked. Further,
     it's generally hard to even get a complete, accurate statement
     of what correctness for a variety of reasons: specification size,
     ambiguity, corner cases or outright  specification mistakes.
     Specfications are not magic, and they have bugs just like code does
     (and for similar reasons) --- you have already seen an example
     in the Broadcom document where their description of their own
     hardware incorrect.

     However, while a constructive expression of correctness is hard, if
     you have multiple implementations of the same interface, *detecting*
     incorrectness is often easy: for each input or test you have simply
     run each different implementation on the same input and check if
     the output is identical.  If they differ, and the input was legal,
     then at least one is wrong.

     Note that if all implementations agree this does not mean they
     are correct.  They could all have the same mistake.  But, note,
     also that in such cases, the other approach of checking against a
     specification would also likely miss the error (because the same
     conceptual mistake could easily cause issues with the specification
     as well).

     But, in general, there is no panacea for correctness --- if the
     code matters, do everything you can!  One nice thing about easy
     approaches is you can do a lot more of them.

     If there's one thing you learn in this class, it's hard to
     do better than routinely checking your code in such a way.
     Cross-checking appears in many places in computer systems (e.g.,
     "n-version programming")  and, also outside them.  In a sense, you
     could argue that cross-checking ("same input = same result?") is
     a key reason that the natural sciences have seen such a massive
     increase in effectivess over the past few centuries, where they
     call it "reproducibility".

----------------------------------------------------------------------
#### Background: making a fake pi implementation

In order to make testing easy, we want to be able to run your r/pi code
--- unaltered --- on your Unix laptop.  This might seem implausible,
since the code was written to run on the r/pi.   However, if as you look
at the code, most of it is C code, which will run the same on the r/pi
and your laptop --- if-statements will work the same as will addition,
assignments, return statements, etc.

The main pi-specific stuff from our code in lab 1 is: 
   1. The assembly code (in `start.S`).  We will be testing the
      `gpio.c` implementations in isolation, so the assembly is not
      relevant.    Thus, we can compile `gpio.c` to run on your Unix
      laptop by simply switching compilers.

  2. The reads and writes to GPIO addresses.  Because we implemented
      `gpio.c` to never access GPIO addresses directly, but instead only
      use `GET32` and `PUT32` we can trivially handle reads and writes
      of GPIO addresses by writing a fake implementation of `PUT32` and
      `GET32`.  For our purposes today its enough to just implement a fake
      memory where at eeach `PUT32(a,v)` call we record that `mem[a]
      = v` and at each `GET32(a)` call, we return `mem[a]` if it exists.

Implementation:

  - To make our `fake-pi.c` code as absolutely simple as possible
    our initial brain-dead implementation has a set of `enum` identifiers
    for each tracked address (you will recognize these addresses from
    `gpio.c`):

            // the locations we track.
            enum {
                gpio_fsel0 = (GPIO_BASE + 0x00),
                gpio_fsel1 = (GPIO_BASE + 0x04),
                gpio_fsel2 = (GPIO_BASE + 0x08),
                ...

    Along with a global variable holding the value for each one:

            // the value for each location.
            static unsigned
                    gpio_fsel0_v,
                    gpio_fsel1_v,
                    gpio_fsel2_v,

  - `PUT32` simply switches on the input address and writes the associated
    global (if any):

        void PUT32(uint32_t addr, uint32_t v) {
            ...
            switch(addr) {
            case gpio_fsel0: gpio_fsel0_v = v;  break;
            case gpio_fsel1: gpio_fsel1_v = v;  break;
            case gpio_fsel2: gpio_fsel2_v = v;  break;
            case gpio_fsel3: gpio_fsel3_v = v;  break;
            ...


  - `GET32` switches on the input address and returns the associated
    value (if any):

        // same but takes <addr> as a uint32_t
        uint32_t GET32(uint32_t addr) {
            unsigned v;
            switch(addr) {
            case gpio_fsel0: v = gpio_fsel0_v; break;
            case gpio_fsel1: v = gpio_fsel1_v; break;
            ...
            case gpio_lev0:  v = fake_random();  break;

       Note we treat most GPIO memory as the same as "regular" memory
       in that every read returns the value of the last write.  However,
       we treat one address differently --- `gpio_lev0` --- since that is
       how code reads the value of an input pin.  Input is controlled by
       the external environment --- if we simply returned the value of
       the last write we would ignore many possible values.  Instead we
       return a pseudo-random value each time, since the environment
       could change.

       Similar in spirit, we could add additional checking to detect
       if people ever read the `set0` or `clr0` addresses (this caused
       various bugs in lab 1).  You're welcome to do so!

To see how this all works:

  1. Look in `1-fake-pi/fake-pi.c` and read the comments.
     For the extension you will modify `PUT32` and `GET32`, but for now
     just understand how they work.

  2. Before you start, run `make` in `1-fake-pi/` and make sure everything
     compiles.  Note: the code will use your `gpio.h` and `gpio.c`
     from lab 1.

  3. You will now have `1-fake-pi/prog-1-blink`, `1-fake-pi/prog-2-blink`,
     and `1-fake-pi/prog-3-input`
     --- you can run them and see the `PUT32` and `GET32` calls they
     perform.

  4. An easy way to compare these results to your partner is to compute
     a "checksum" of them that reduces the arbitrary output to a single
     integer.  We do so using the standard Unix `cksum` program.

  5. If you then look in `1-fake-pi/tests` you will see a more complete set
     of tests.

The `Makefile` in `1-fake-pi/tests` automates the process, but to understand
how this works, let's run the `1-blink` from last lab using `fake-pi`:

            % cd labs/3-cross-check/1-fake-pi/tests
            % make
            % ./prog-1-blink > prog-1-blink.out
            % cksum prog-1-blink.out

  1. `./prog-1-blink > prog-1-blink.out`: should run without crashing and,
     importantly, print out the values for each `PUT32` and `GET32`
     in the exact order they happened and store them in `1-blink.out`.
  2. `cksum prog-1-blink.out`: computes and prints the checksum of the stored output.
     You can compare this to your partner(s).
  3. If these values match, you know your code worked the same as your partner's.
  4. Now as you do all the rest, post your results to the newsgroup so everyone can compare.
  5. If everyone matches, and one person got it right, we've shown that
       everyone has gotten it right (at least for the values tested).

----------------------------------------------------------------------
#### Part 1. Check your code against everyone else 

To summarize the above fake-pi description: Given the `GET32` and `PUT32`
modifications above, a simple, stringent approach is to check that two
`gpio` implementations are the same:

  1. They read and write the same addresses in the same order with
     the same values.
  2. They return the same result.    (For our implementations we
     did not return any result, so this just means that your code never
     crashes.)


If all checks pass then we know that both implementations are equivalent
in how they read or write device memory --- at least for the tested inputs.

You won't write much code for this part, most of the work will be
comparing different `PUT32`, `GET32` and `DEV_VAL32` traces for the
tests given in `1-fake-pi/tests` to see what the differences are from.

##### Setup.

First things first:

  1. In general: you don't want to break working code.  So before making
     changes `cd` into `1-fake` and copy your gpio.c from
     last lab:

            % cd 1-fake
            % cp ../../2-labs/code/gpio.c .

  2. Change `gpio_read` in `gpio.c` so that on the non-error case, it
     calls `DEV_VAL32` with its return value and returns that result.
     This lets us check more stringently.

     So for example:


            int gpio_read(unsigned pin) {
                if(pin >= 32)
                    return -1;
                ...
                return x;
            }

     Becomes

            int gpio_read(unsigned pin) {
                if(pin >= 32)
                    return -1;
                ...
                return DEV_VAL32(x);
            }


  3. Make sure you can compile without errors.

            # in 1-fake-pi
            % make 
       
  4. Change into the `tests` subdirectory and run a single test.

            % cd tests
            # should run a single test
            % make run
            # test should pass.
            % make check

            checking 0-gpio-write-17:
            about to emit a new test output <0-gpio-write-17.test>:
                ./0-gpio-write-17.fake > ./0-gpio-write-17.test
            about to compare...
            Success!  Matched!

  5. If you look in the `Makefile` in tests it describes how to 
      run the other tests.  


##### Workflow

The tests are organized in increasing difficulty to debug.   
  - Tests with a `0-` prefix are the easiest (start there) and just contain
    a single legal call to a `gpio.c` routine.
  - Tests with a `1-` prefix are a bit harder and systematically run all
    legal pins less than 32.
  - Tests with a `2-` and `5-` prefix are more difficult since they test
    many pins, both
    legal and illegal.

  - Tests with a `prog-` prefix are full program tests (taken from last lab).
  - Tests with a `act-` prefix are run the act led tests.


The `Makefile` in `1-fake-pi/tests` has a set of targets to automate
the process.

  - The files specified by the `TEST_SRC` variable at the top of `1-fake-pi/tests/Makefile` 
    determine which tests get run.  For example, to run all simple tests:

            TEST_SRC := $(wildcard ./[0]-*.c)

    You could change `[0]` to `[1]` to run the next batch of tests, etc.
    `Makefile` variables are like regular variables in that the last
    write wins.  You don't have to comment out previous writes.

  - `make run`: will run all the specified tests.
  - `make emit`: will run all the specified tests and save the output to a `.out` file.
     (e.g., running `1-blink` will produce `1-blink.out`).  You can open this file like you
     would any, but using the `more` or `cat` command is easier / faster.
  - `make cksum`: will run all the specified tests and reduce their entire output to a single
     integer using the `cksum` program.  You can quickly determine if you get the
     same result as your partner(s) by just comparing this integer.
  - `make checkoff` will compute all the checksums.
  - `make check` compares to old .out files.

You should compare to your partner and work through the tests.

##### Making  code behave the same on illegal inputs

The `0-` and `1-` tests  work on legal inputs.  You should get these
working.  The next set (`2-*.c`) check illegal pin inputs as well.
If we don't state how to handle them, it's easy to get different results.

To keep things simple, you should check if a pin is larger than 32
and, for `void` routines, simply check and return at the start of 
the routine:

        // 47 is the internal led pin
        if(pin >= 32 && pin != 47)
            return;

(So: At the start of `gpio_set_on`, `gpio_set_off`, `gpio_set_input`,
`gpio_set_output`.)

And for non-void, such as `gpio_read`, check and return `-1`:

        // 47 is the internal led pin
        if(pin >= 32 && pin != 47)
            return -1;

We would normally print and error and crash, but at the moment we don't
even have `printk` so simply return.

Note: no matter what, we definitely need to check for illegal pins
since they are being used to compute an address that we read and write
to (i.e., they determine the value passed to `GET32` and `PUT32`).
Since we are running without memory protecton, such invalid accesses
are extremely bad since they can silently corrupt data we use or issue
weird hardware commands.

##### Workflow

In general, when you work on tests:

  - `make run`: will compile and run the tests.  This is good
    to check for a crash.

  - `make emit` will emit a `.out` file for each configured
     test.  This is the output we run `cksum` on later, and 
     is also the used for regression testing.
  - `make check` will rerun tests and compare there output to 
    previously emitted .outs.  You want to do this after any
    modification to `gpio.c` so that you can make sure old
    functionality doesn't change.

  - `make cksum` will run `cksum` on every `.out` so that
    you can easily compare to other people.


----------------------------------------------------------------------
#### Step 2: implement `gpio_set_function` and cross-check it 

As you've probably discovered, debugging without even `printf` is a
hassle.  Before we do a bunch of devices in later labs, let's implement
the `GPIO` code `printf` working so that it's a bit easier.

 - For historical reasons we call our kernel's `printf` "`printk`" both
   because it's running at privileged level (and so errors can crash
   the machine versus an application segmentation fault) and because
   our implementation isn't quite `printf`.

This is the one step where you write some code.  But it's mainly just
adapting the GPIO code you already implemented.

The header `$CS140E_2025_PATH/libpi/include/gpio.h` in today's 
lab gives the definition:

    // set GPIO function for <pin> (input, output, alt...).  
    // settings for other pins should be unchanged.
    void gpio_set_function(unsigned pin, gpio_func_t function);

This is just a more generic version of your `gpio_set_output` and
`gpio_set_input`.    It takes an `enum` constant telling it how to
configure the pin (the Broadcom document pages you used for the last
lab tell you what they mean).

What to do:

  0. Do all edits to your `gpio.c` you copied from lab 1 to `1-fake-pi`
  1. Adapt your `gpio_set_output` code to bitwise-or the given flag.
  2. Error check not just the input `pin` but also the `function` 
      value.

Checkoff:
   1. Make sure the `5-tests*.c` are equivalant to other people.
   2. Rewrite your `gpio_set_input` and `gpio_set_output` to call 
      `gpio_set_function` and then verify you get the same checksums.
      (See a pattern?)

   3. Checking that `printk` now works for real; if you go into
      `3-cross-check/check-hello` and type `make` it produce a `hello.bin`
      that it can run successfully.
      If so, congratulations!  It is using your `gpio.c`

-------------------------------------------------------------------
#### Step 3: implement the act led (pin 47).

The pi has an led on its board (the "act" led) that you can control 
by writing GPIO pin 47.

For `gpio.c`:
  - extend `gpio.c:gpio_set_func` to handle 47.
  - extend `gpio.c:gpio_set_on` and `gpio.c:gpio_set_off`
  - `gpio_read` should still give an error for any pin greater than 32.

For `fake-pi.c`:
  - You will have to extend `fake-pi.c` to have additional clear
    (`gpio_clr1`), set (`gpio_set1`) and function select location
    (`gpio_fsel4`).

  - extend `fake-pi.c:GET32`: to handle reads of the new function select location.
  - extend `fake-pi.c:PUT32`: to handle writes to the new clear and set locations.
  - do not add calls to initialize the new variables in `notmain` using
    random!  It will throw off the values.  You'll notice `gpio_fsel4_v` is
    set to `~0`.

For testing:
  - Enable the act tests:

        # 1-fake-pi/tests/Makefile
        TEST_SRC := $(wildcard ./act-*.c) 

----------------------------------------------------------------------
----------------------------------------------------------------------
## Final checkoff for part 1.

Now that you're done, for checkoff:

  1. As described at the start of the README, 
     in `1-fake-pi/test/Makefile`  uncomment the line:


            # 6. then do everything.
            TEST_SRC := $(wildcard ./[0-5]-*.c) ... [more] ...

     So we do all the tests.

  2. run `make  checkoff` which will compute a checksum of all
     the checksums.  You should get a total of 29 files, and the
     checksum should be `525118589`.




----------------------------------------------------------------------
#### Part 2: Do similar tracing on the pi (`2-trace`)

***Note: for the `prog-hardware-loopback.c` you'll need to run a jumper
between pins 9 and 10 (it sets and reads).***

This uses the tracing trick from the `PRELAB`.  You should look at that
implementation if you haven't already.

Implement the code in `2-trace`:

  - `trace-simple.c`: implement `__wrap_PUT32` and `__wrap_GET32`
  - `trace-notmainc.c: if you want to get fancy implement this
    so you can run raw pi programs in tracing mode.
  - extend `tests/Makefile` to also handle get32 and put32.  You
    should just change your trace library to call the wrappers for
    `PUT32` and `GET32`.

As with `1-fake-pi` start working through the tests in `2-trace/tests`.

###### checkoff

Note, that initially you will be using our `gpio` implementation in
`libpi`.  When you finish the tracing above do, emit the `out` files
and then drop in your gpio and make sure you get the same answer.

   1. `make emit`.
   2. `make check` to make sure it passes (this compares the current run to 
       the output files emitted in (1)).
   3. Change `libpi/Makefile` to use your `gpio.c` instead of ours by changing
      `SRC = src/gpio.c` and removing the `staff-objs/gpio.o` from `STAFF_OBJS`.
       These steps are described in the `libpi/Makefile` comments.

       ***To make debugging easy: before doing anything else, check that
      running `make` in `3-cross-check/check-libpi` doesn't break: for
      `hello-bin.bin` should print "hello" and `act-blink.bin` should
      blink the small green led on the pi itself.***

   4. Now verify tracing gives the same values: `make check`: you should get the same results.

----------------------------------------------------------------------
#### Extension: simulator validation

Modify the `fake-pi.c` implementation to set memory to the actual values
on the pi when called with `-initial`.  You should then check that you
get the same results when run on the same program.  Note: you will have
to do something about the tracing start/stop calls.

----------------------------------------------------------------------
#### Extension: Implement a better version of memory that uses an array.

Our crude memory in `fake-pi.c` is relatively simple to understand, but
very rigid since it uses a global variable for each address.  Make a copy
of code (`cp -r code code-new`) and reimplement memory using an array.
When you rerun all the tests nothing should change.

----------------------------------------------------------------------
#### Extension: write other tests.

We'll give an extension credit if you can write a test you that detects
a new difference that the provided tests miss.

-------------------------------------------------------------------------
#### Addendum: the power of fake execution + equivalance.

It doesn't look like much but you've implemented a fancy approach to
correctness that is --- hard to believe --- much beyond what almost
anyone does.     There are two pieces to what you've done:

   1. Taking low-level code made to manipulate hardware directly and
      running it without modification in a fake environment.   This
      approach to lying (more politely called "virtualization") is
      powerful and useful in many domains.  In an extreme case you have
      virtual machines, such as VMware, which can take entire operating
      systems designed to run in god-mode on raw hardware (such as your
      laptop) and instead run them as user-processes that deal with fake
      hardware without them realizing it.

   2. Showing code equivalance not by comparing the code itself (which
      is hard when code differs even trivially, much less dramatically)
      but instead by checking that it does equivalant reads and writes.
      Checking memory-access equivalance gives us a simple, powerful
      method to mechanically check code correctness.    In our case, it
      lets us show that you are equivalant to everyone else by comparing
      a single 64-bit number.  Even better: if even one impementation
      is correct, we have trivially shown they all are --- at least on
      the runs we have done --- no matter how weirdly you wrote the code.

Showing exact equivalance of a specific run is easy.   (A `strcmp` of the
output is sufficient.)   It does have a couple of challenges.

   1. It only shows equvalance on the inputs we have tested.  It cannot
      show that there is not some other input that causes a non-equivalant
      execution.  We *can* signficantly extend this by checking the code
      symbolically (essentially running it on constraints expressing
      all possible values on a path vs just running it on a single set
      of concrete values), but this is beyond the scope of the course.
      My student David Ramos has two great papers on this appraoch.

   2. It will semantically equivalant runs that have superficial
      differences.  For example, consider code where one implementation
      first sets GPIO pin 19 to an output pin, and then pin 20 while
      another does the reverse.  Intuitively these are the same, since
      it does not matter which order you set these pins, but since these
      reads and writes will be in different orders we will reject these.
      The problem with resolving this false rejection is that we need
      to know more semantics of what we are checking.   While encoding
      such semantics in this exact case isn't hard, doing so in general
      is not always easy.  And it always requires some more code (and,
      of course, this code can be wrong).  There is an art to flexibly
      specifying such differences which, unfortunately, we lack the time
      to get into.  One easy (but a bit imperfect) hack in our domain
      is to consider reads and writes to different r/pi devices to be
      independent (this is not always true) or multiple reads or writes
      of the same location to be equivalant to a single read or write
      (again, not always true).

      In the interest of simple and fast we will insist on exact
      equivalance, but you are more than welcome to do something more
      fancy and we'll try to factor that into your grade.
