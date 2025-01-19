# need the makefile to give an error if this doesn't exist
PROGS := timer.c
COMMON_SRC := interrupts-asm.S 
START := staff-start.o
MEMMAP := ./memmap

# uncomment if you want it to automatically run.
RUN=1

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-v3
