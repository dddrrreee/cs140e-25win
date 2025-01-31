LIBNAME = libpi-fake.a
BUILD_DIR = ./objs

LPP = $(CS140E_2025_PATH)libpi

# all the .c files used to generate fake programs on Unix.
# 1. get all the the routines from current directory.
LIB_SRC  = fake-pi.c  fake-random.c  pi-random.c
# 2. pull in different libpi files.
LIB_SRC  += $(LPP)/libc/printk.c
LIB_SRC  += $(LPP)/libc/putk.c
LIB_SRC  += $(LPP)/libc/putchar.c
LIB_SRC  += $(LPP)/staff-src/clean-reboot.c

# 3. pull in your gpio.c and uart.
ifdef CS140E_STAFF
    LIB_SRC  += $(LPP)/staff-private/gpio.c 
    LIB_SRC  += $(LPP)/staff-private/uart.c 
else
    # your code in libpi/src
    LIB_SRC  += $(LPP)/src/gpio.c 

    # your uart from this lab.
    LIB_SRC  += ../1-uart/uart.c

    # or can use the one in libpi
    # LIB_SRC  += $(LPP)/src/uart.c
endif

# all the include locations.
INC +=  -I$(LPP)/include
INC +=  -I$(LPP)/libc
INC +=  -I$(LPP)/staff-src
INC +=  -I.

include $(CS140E_2025_PATH)/libunix/mk/Makefile.lib.template

# file that you can include in a makefile to define INC
all::
	@echo "INC += $(INC)" > includes.mk

clean::
	rm -f includes.mk

