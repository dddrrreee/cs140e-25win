###
#  Don't modify this --- we are updating it.
#
#

DEPS += ./Makefile ./manifest.mk
COMMON_SRC = $(SRC)


BUILD_DIR := ./objs
LIBNAME := libpi.a
START := ./staff-start.o

# set this to the path of your ttyusb device if it can't find it
# automatically
TTYUSB = 

# the string to extract for checking
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|PASS:\|TEST:'

# set if you want the code to automatically run after building.
RUN = 1
# set if you want the code to automatically check after building.
#CHECK = 0

ifdef CS140E_STAFF
ifndef CS140_ACT_AS_STUDENT
STAFF_OBJS += staff-objs/kmalloc.o
#STAFF_OBJS += staff-objs/sw-uart.o
#STAFF_OBJS += staff-objs/interrupts-asm.o      
#STAFF_OBJS += staff-objs/interrupts-vec-asm.o 
#STAFF_OBJS += staff-objs/rpi-thread.o 
#STAFF_OBJS += staff-objs/rpi-thread-asm.o 
#STAFF_OBJS += staff-objs/new-spi.o
endif
endif

DEPS += ./src

# we need these first so that the catchall .o rule in Makefile.robust
# doesn't claim them.
all:: ./staff-start.o

# copy staff-start.o to top level to make it easier to include.
staff-start.o: $(BUILD_DIR)/staff-start.o
	cp $(BUILD_DIR)/staff-start.o .

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed

clean::
	rm -f staff-start.o
	rm -f staff-start-fp.o
	make -C  libc clean
	make -C  staff-src clean

.PHONY : libm test
