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

# copy your gpio here!
LIB_SRC  += gpio.c 

# all the include locations.
INC +=  -I$(LPP)/include
INC +=  -I$(LPP)/libc
# INC +=  -I$(LPP)/staff-src
INC +=  -I.

include $(CS140E_2025_PATH)/libunix/mk/Makefile.lib.template
# include ./Makefile.lib.simple


# file that you can include in a makefile to define INC
all::
	@echo "INC += $(INC)" > includes.mk

gpio.c:
	$(error "copy 2-gpio/code/gpio.c into this directory");

checkoff:
	make -C tests checkoff

clean::
	rm -f includes.mk
