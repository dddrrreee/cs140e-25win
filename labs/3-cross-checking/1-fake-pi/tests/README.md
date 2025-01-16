### fake pi tests.

Look in the `Makefile` for explanation.

In `Makefile`, if you set `TEST_SRC` to:

    TEST_SRC := $(wildcard ./[0-5]-*.c) $(wildcard ./prog-*.c)

you should have the same cksums as everyone else when you run 

    % make checkoff
