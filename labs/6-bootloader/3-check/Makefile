# need the makefile to give an error if this doesn't exist
PROGS := check.c
# COMMON_SRC :=  sim-get-code.c sim-put-code.c
    
# staff thread code.  just start w/ this.  then drop in yours.
STAFF_OBJS += $(CS140E_2025_PATH)/libpi/staff-objs/staff-rpi-thread.o
STAFF_OBJS += $(CS140E_2025_PATH)/libpi/staff-objs/staff-rpi-thread-asm.o

INC += -I. -I../2-pi-side.staff -I../1-unix-side.staff
LIB_POST += $(CS140E_2025_PATH)/lib/libgcc.a

# hmm.  not sure about this.
BUILD_DIR = ./objs

RUN=1

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed
