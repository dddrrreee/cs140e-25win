# NOTE: "make check" is extremely weak for today --- 
# we don't know what is on your microsd so can't check
# in general.  its mainly useful so you can run
# a bunch of tests automatically and get notified if
# something failed.
#
# when developing: you should probably run "make" 
# and look at the output to make sure it makes sense.

# uncomment COMMON_SRC and comment out STAFF_OBJS
# COMMON_SRC += mbr.c
STAFF_OBJS +=  staff-mbr.o

# uncomment COMMON_SRC and comment out STAFF_OBJS
# COMMON_SRC += fat32.c
STAFF_OBJS +=  staff-fat32.o

# the tests in decreasing order of difficulty.

# PROGS := $(wildcard tests/[3]-*.c)
# PROGS := $(wildcard tests/[2]-*.c)
PROGS := $(wildcard tests/[1]-*.c)
PROGS := $(wildcard tests/[0]-*.c)

# since each takes so long, break them out.
# when doing part 2 just build one at a time
# to save time.
# PROGS := tests/2-fat32-mk.c
# PROGS := tests/2-fat32-read.c
# PROGS := tests/2-fat32-hash-read.c  
# PROGS := tests/2-fat32-ls.c  
# PROGS := tests/2-fat32-jump.c	   

# for checkoff don't do r/w
ALL_PROGS := $(wildcard tests/[012]-*.c)

CFLAGS_EXTRA  = -Iexternal-code

# a list of all of your object files.
COMMON_SRC += pi-sd.c mbr-helpers.c fat32-helpers.c fat32-lfn-helpers.c external-code/unicode-utf8.c external-code/emmc.c#  external-code/mbox.c 

# external-code/bzt-sd.c 

TTYUSB = 
BOOTLOADER = my-install
RUN = 1

EXCLUDE ?= grep -v simple_boot
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|PASS:\|TEST:'
include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust

checkoff:
	make -C ./ "PROGS=$(ALL_PROGS)"

clean::
	rm -f tests/*.list tests/*.o tests/*.elf tests/*~ tests/*.bin
	make -C external-code clean
