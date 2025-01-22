# need the makefile to give an error if this doesn't exist
PROGS := gprof.c

# use our own private copy so can modify it.
COMMON_SRC += interrupts-asm.S

# for kmalloc (you'll build this in a couple labs)
STAFF_OBJS = staff-kmalloc.o

# uncomment if you want it to automatically run.
# RUN=1

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-v3
