# uses one of the standard built-in makefiles.   you just
# need to define a few variables

PROGS := trace-foo.c
COMMON_SRC := foo.c

# flags for the linker
LDFLAGS = --wrap=foo  

# bootloader to use.
BOOTLOADER = pi-install

# automatically run the .bin files
RUN=1

# set this if you need to give the device for your pi
# TTYUSB = 

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed
