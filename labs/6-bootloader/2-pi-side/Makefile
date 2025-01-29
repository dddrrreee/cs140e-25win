# need the makefile to give an error if this doesn't exist
PROGS := main.c
COMMON_SRC := 

# hmm.  not sure about this.
BUILD_DIR = ./objs
START := $(BUILD_DIR)/boot-start.o

# we can't bootload the bootloader.
RUN=0

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed

all:: kernel.img

clean::
	rm -f $(START) kernel.img

kernel.img: main.bin
	cp main.bin kernel.img
