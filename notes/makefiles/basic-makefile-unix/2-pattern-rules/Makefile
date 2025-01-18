# add a pattern matching rule for .o's  instead of manually 
# specifying each of them.
#
# the safety checking actually makes this version a bit longer 
# than our previous Makefile :)  however, this approach works 
# much better in practice since effort doesn't grow O(n) with 
# the number of files.

# specify flags to pass to compiler.
CFLAGS := -O -Wall

all: main test

main: main.o a.o b.o c.o
	# note we could have used $$^ ($^)
	$(CC) a.o b.o c.o main.o -o main

# pattern matching rule that generates a .o from a .c in the same
# location.
#
# we print out what we are doing (using a comment, which is 
# a bit weird!) just so we can see it --- there's no need 
# for this in general.
#
# as you can see from the print:
#   - $< is the input .c file name
#   - $@ is the output .o file name.
%.o: %.c header.h
	# special vars: $$@ = %.o = $@ and $$< = %.c = $<
	$(CC) $(CFLAGS) -c $<  -o $@

###############################################################
# same as previous <Makefiles>

test: main
	./main > out
	diff out main.ref
	@echo "makefile: program 'main' passed test"
	@rm -f out

# same as before
clean:
	rm -f main out *~ *.bak *.o *.d

.PHONY: all clean test
