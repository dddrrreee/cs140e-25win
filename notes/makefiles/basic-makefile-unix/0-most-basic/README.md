## 0: basic unix makefile.

[Very simple makefile](./Makefile) that compiles a set of C files that
include a header file `header.h` into a executable `main` and then
compares its output to a reference output.

It manually specifies dependencies and doean't use
incremental compilation.  Advantages:
  - Explicit, manual dendencies = impossible to miss what is going on.
  - Just recompiing everything = don't need any `.o` rules etc.

To run:

        % make

Should see:

        # main: rule
        cc a.c b.c c.c main.c -o main
        # test: rule
        ./main > out
        diff out main.ref
        makefile: program 'main' passed test

If we go though and touch each of the `.c` and `.h` files (i.e., update
their modification times), you should see `main` is correctly recompiled:

        % touch a.c
        % make main # just compile, don't test
        # main: rule
        cc a.c b.c c.c main.c -o main
        % make main
        make: 'main' is up to date.

        % touch b.c
        % make main
        # main: rule
        cc a.c b.c c.c main.c -o main
        % make main
        make: 'main' is up to date.

        % touch c.c
        % make main
        # main: rule
        cc a.c b.c c.c main.c -o main
        % make main
        make: 'main' is up to date.

        % touch header.h
        % make main
        # main: rule
        cc a.c b.c c.c main.c -o main
        % make main
        make: 'main' is up to date.

One downside for large projects: recompiling everything each time. 
