#
# simple example of building a fake-pi implementation.
#

##################################################################
#  the different fake pi implementations.  0 is the most basic.  
#  2 is the fanciest.
#  switch the variable to run each.
# last writer wins: start with 0-libpi-fake
FAKE_VERSION = 1-libpi-fake
FAKE_VERSION = 0-libpi-fake
FAKE_VERSION = 2-libpi-fake

MAKEFLAGS += --no-print-directory

# can uncomment these for 1-libpi-fake or 2-libpi-fake
#PI_PROGS += test-demand.c
#PI_PROGS += test-assert.c

# use this so we can pass the same set of programs to both makefiles.
PI_PROGS += hello.c null-ptr.c 

all: fake-pi

pi:
	make -f Makefile.pi "PI_PROGS=$(PI_PROGS)"

fake-pi:
	make -f Makefile.fake-pi "PI_PROGS=$(PI_PROGS)" "FAKE_VERSION=$(FAKE_VERSION)"

# we should be able to add as dependencies?
both:
	make -C ./ fake-pi
	make -C ./ pi

clean:
	make -s -f Makefile.fake-pi clean
	make -s -f Makefile.pi clean
	make -s -C $(FAKE_VERSION) clean
	make -s -C 0-libpi-fake clean
	make -s -C 1-libpi-fake clean
	make -s -C 2-libpi-fake clean
	rm -f *~ *.bak

.PHONEY: both fake-pi pi clean
