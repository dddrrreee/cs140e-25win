ALL_PROG := $(wildcard tests/[012]*-test.c)
RUN_PROG := $(wildcard tests/[3]*-test.c)

all: checkoff

checkoff:
	make -f ./Makefile PROGS="$(ALL_PROG)" clean
	make -f ./Makefile PROGS="$(ALL_PROG)" check
	make -f ./Makefile PROGS="$(RUN_PROG)" run

