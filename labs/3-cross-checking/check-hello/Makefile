# this is the makefile template we use --- a bunch of stuff
# happens in the included makefile.  we will simplify, but
# for the moment, time-pressure.

# the driver programs: these all must have a notmain.
PROGS := hello.c 

# COMMON_OBJS := ../1-fake-pi.staff/gpio.o
COMMON_OBJS := ../1-fake-pi/gpio.o

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

# uncomment if you want it to automatically run.
RUN=1

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-v3

