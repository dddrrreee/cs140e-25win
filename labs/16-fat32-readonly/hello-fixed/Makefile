PROGS := hello-f.c 

# use our own custom cstart
COMMON_OBJS := loader-cstart.o
# override memmap: look inside to see where it's linked at.
MEMMAP = ./memmap.fixed
# override start 
START = loader-start.o

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# uncomment if you want it to automatically run.
RUN=0

# include $(CS140E_2024_PATH)/libpi/mk/Makefile.template-v2
include $(CS140E_2024_PATH)/libpi/mk/Makefile.template-v3

# add more actions to clean
clean::
	rm -f *.o
