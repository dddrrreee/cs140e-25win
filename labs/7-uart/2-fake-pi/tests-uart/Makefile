# TEST_SRC are all the files that get run.
#    - you should start with the easiest first --- just do a single
#      one of the 1-* files (these do single call)
#    - then do all of them and see that you get the same "make checkoff"
#      as other people
#    - then do 2
#    - then do 5
#    - then do prog-*
MAKEFLAGS += --no-print-directory

PROGS = 0-uart-init.c
PROGS += 0-uart-putc.c  
PROGS += 0-uart-getc.c  
PROGS += 1-hello.c

ALL_TESTS = 0-uart-init.c 0-uart-putc.c  0-uart-getc.c  1-hello.c
ALL_OUTS = $(ALL_TESTS:.c=.out)

LIBPI_FAKE = ../libpi-fake.a

INC  =  -I../  -I.
INC +=  -I$(CS140E_2025_PATH)/libpi/include
INC +=  -I$(CS140E_2025_PATH)/libpi/libc
INC +=  -I$(CS140E_2025_PATH)/libpi/staff-src
CFLAGS = $(INC) -g -Wall -Wno-unused-function -Wno-unused-variable

LIBS += $(LIBPI_FAKE)

include $(CS140E_2025_PATH)/libunix/mk/Makefile.unix

libs::
	make -C .. 

checksum: 
	make -C . emit
	@echo "checking output for: $(ALL_TESTS)"
	ls $(ALL_TESTS)
	@echo "checking outfiles exist: : $(ALL_OUTS)"
	ls $(ALL_OUTS)
	@echo "individual checksums = ";
	@cksum $(ALL_OUTS) | sort -n 
	@echo "checksum(checksum) = ";
	@cksum $(ALL_OUTS) | sort -n | cksum
