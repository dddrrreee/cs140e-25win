## 4-auto-dependency-gen

[Makefile with auto-dependencies](./Makefile):
We now use the `-MMD` flag to automatically generate dependencies rather
than hardcode the headers that each .c depends on (which can go wrong
in bad ways).

We are well on our way to having a makefile that you can use for
most cases in your career:
  - compile all the `.c` files in a directory
  - into an executable in the same directory
  - with automatic dependencies.

You shoudl test that:
  - `touch a.c`
  - `touch b.c`
  - `touch c.c`
  - `touch header.h`
All work as expected.

Also: look in the `.d` files to see why they work (it should be obvious
how you'd write them by hand, since that is what we did in `0-most-basic`
and `1-incremental-compilation`).
