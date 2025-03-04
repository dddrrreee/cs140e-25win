# make a custom executable that:
#   1. we can call from the fat32 file system:
#      - that does *not* do r/pi initialization (e.g.,
#        uart) since we are already running.
#      - will return rather than reboot when done (so
#        we can keep running).
#   2. you should copy it to the microSD.
#   3. look inside the memmap.fixed linker script to 
#      understand the layout so can call it from your
#      fat32 test case.
PROGS := hello-f.c 

# use our own custom cstart
COMMON_SRC := loader-cstart.c
# override memmap: look inside to see where it's linked at.
MEMMAP = ./memmap.fixed
# override start 
OUR_START = loader-start.S

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# uncomment if you want it to automatically run.
RUN=0

# include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed
include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust

# add more actions to clean
clean::
	rm -f *.o
