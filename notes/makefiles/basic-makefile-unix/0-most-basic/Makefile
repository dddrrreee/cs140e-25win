# makefile 0.
#
# a very simple-minded Makefile that hardcodes all dependencies
# and:
#  1. produces an executable file <main> by compiling
#     C files <a.c>, <b.c>, <c.c>, <main.c> and a header 
#     file <header.h> without intermediate .o files.
#  2. automatically checks that the output of <main> is the same 
#     as a reference <main.ref>
#

# we have <all> listed as the first makefile target.  it will
# be what make attempts to build if invoked without 
# a specific target (e.g., as "make" vs "make clean").
#
# we are telling make that <all> depends on <main> and <test>
# so it will attempt to build them.
all: main test

# hardcode that <main> depends on <a.c> <b.c> <c.c> <main.c> and
# <header.h>
#
# what this means mechanically: if the modification time of any
# of these files changes so it's more recent than the modification
# time of <main> (or if <main> doesn't exist) the rule will execute.
#
# easy test is to run the following and check:
#  % make clean  # removes <main>
#  % make        # compiles  <main>
#  % make        # should not re-compile <main>
#  % touch header.h  # change the modification time on <header.h>
#  % make        # will re-compile <main>
#
# note: we don't ever compile these files to a .o we just
# compile them as a big single command using gcc.  this is 
# the simplest, easiest way to do this.  for small projects
# there's no reason to mess with incremental compilation.
main: a.c b.c c.c main.c header.h
	# main: rule
	$(CC) -Wall -Og a.c b.c c.c main.c -o main

# check <main> produces the same output as reference
# output saved in <main.ref>:
#  1. run <main> and put the output in file <out>;
#  2. check if they differ using <diff>
#
# This will rerun each time you run `make`
test: main
	# test: rule
	./main > out
	diff out main.ref
	@echo "makefile: program 'main' passed test"
	@rm -f out

# remove outputs and temporary files
clean:
	rm -f main out *~ *.bak

# tell `make` that these rules don't produce a file
# of the same name (they are phoney -- fake --- target
# names)
.PHONY: all test clean 
