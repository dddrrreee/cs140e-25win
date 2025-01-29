PROGS += 0-read-test.c 
PROGS += 1-read-test.c 
# note: you can't do "make check" for this one.
# PROGS += 0-tty-test.c
COMMON_SRC = 

ifdef CS140E_STAFF
COMMON_SRC += $(CS140E_STAFF_2024_PATH)/libunix/staff-find-ttyusb.c
COMMON_SRC += $(CS140E_STAFF_2024_PATH)/libunix/read-file.c
CAN_EMIT=1
endif

RUN=0

include ../../../libunix/mk/Makefile.unix
