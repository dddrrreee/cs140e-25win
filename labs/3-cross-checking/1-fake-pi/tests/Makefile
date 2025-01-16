###############################################################################
# testing makefile.
#  1. assign the tests to run in <TEST_SRC>.  e.g.:
#      - run a single test <0-gpio-write-17.c>:
#           TEST_SRC := 0-gpio-write-17.c
#      - run all tests with 0-* prefix:
#           TEST_SRC := $(wildcard ./[0]-*.c)
#      see more below.
#   
#  2. test commands (see bottom of Makefile and <Makefile.test>
#       - "make run" it will run all programs in <TEST_SRC>
#       - "make emit" it emit .outs for all programs in <TEST_SRC>
#       - "make check" it will check all programs in <TESTS_SRC> against their .out
#       - "make cksum" it will compute checksums for all programs in <TEST_SRC>
#       - "make checkoff" it will compute a checksum of checksums for <TEST_SRC>
#
# for test:
#   - the number in front of the test is sort-of difficulty.
#   - easiest are 0-*.c tests.  
#   - next are (1-*.c), etc.

# what to do:
#   - test based on how easy it is to figure things out.
#   - so first make sure a single easy test.  if you match your
#     partner, do a few more.
#   - then do all the 0-*.c tests.
#   - then do all the 1-*.c tests.
#   - etc.
#
# makefile variables are last writer wins, so you can work through the 
# following, uncommenting as you get further and further:
#
# note: you can change these to do assign to TEST_SRC using
# "+=" rather than ":=" if you want to be cumulative.
# e.g.:
#   TEST_SRC += $(wildcard ./[1]-*.c)

# 1. do a single 0-*.c test file since easy.
TEST_SRC := 0-gpio-write-17.c

# 2. do all the 0-* test files (uncomment)
# TEST_SRC := $(wildcard ./[0]-*.c)

# 3. then do the 1-*.c tests (uncomment)
# TEST_SRC := $(wildcard ./[1]-*.c)

# 4. do the act tests.  you'll have to add addresses
# to fake-pi.c
# TEST_SRC :=  $(wildcard ./act-*.c) 

# 6. then do everything.
# TEST_SRC := $(wildcard ./[0-5]-*.c) $(wildcard ./prog-*.c)  $(wildcard ./act-*.c) 

###############################################################################

# so it doesn't make noise.
MAKEFLAGS += --no-print-directory

# fake libpi
LIBPI_FAKE = ../libpi-fake.a

TEST_OUT := $(TEST_SRC:.c=.out)
FAKE_PROGS := $(TEST_SRC:.c=.fake)

INC =  -I../  -I.
INC +=  -I$(CS140E_2025_PATH)/libpi/include
INC +=  -I$(CS140E_2025_PATH)/libpi/libc
INC +=  -I$(CS140E_2025_PATH)/libpi/staff-src
CFLAGS = $(INC) -O -Wall -Wno-unused-function -Wno-unused-variable

DEPS = $(LIBPI_FAKE) $(FAKE_PROGS) 

all: $(LIBPI_FAKE) $(FAKE_PROGS)  run

# force checking libpi-fake.a
$(LIBPI_FAKE): FORCE
	@make -C .. 
FORCE:;

%.fake: %.c  $(SRC) $(LIBPI_FAKE)
	$(CC) $(CFLAGS) $< $(SRC) -o $*.fake $(LIBPI_FAKE)

# i'm not sure this actually makes things better.
emit: $(DEPS) $(FAKE_PROGS)
	@make -f ./Makefile.test emitall "TESTS=$(FAKE_PROGS)"
run: $(DEPS) $(FAKE_PROGS)
	@make -f ./Makefile.test runall "TESTS=$(FAKE_PROGS)"
check: $(DEPS) $(FAKE_PROGS)
	@make -f ./Makefile.test checkall "TESTS=$(FAKE_PROGS)"
cksum: $(DEPS) $(FAKE_PROGS)
	@make -f ./Makefile.test cksumall "TESTS=$(FAKE_PROGS)"

print:
	echo "$(TEST_OUT)"

checkoff: $(DEPS)

	@echo "all files we are going to checksum (TEST_OUT):";
	@ls $(TEST_OUT)
	@echo -n "total files = ";
	@ls $(TEST_OUT) | wc  -l
	@echo -n "checksum of cksum command <ls TEST_OUT| sort -n | cksum> =\n   ";
	@cksum $(TEST_OUT) | sort -n | cksum 

cleanall: clean
	rm -f *.out  *.raw

clean:
	rm -rf *.fake *~ *.o Makefile.bak  

.PHONY: all clean  cleanall emit run check cksum ref
