# you'll implement <mini-watch.c> later in lab
COMMON_SRC += mini-watch.c
COMMON_SRC += mini-step.c


# support dynamic exception registering.
COMMON_SRC += staff-full-except.c 

# you'll do these next week.
STAFF_OBJS := staff-full-except-asm.o  
STAFF_OBJS += staff-switchto-asm.o
STAFF_OBJS += $(CS140E_2025_PATH)/libpi/staff-objs/kmalloc.o

# define this if you need to give the device for your pi
TTYUSB = 
# if your bootloader has issues just use pi-install.
BOOTLOADER = pi-install
BOOTLOADER = my-install

# set to 0 if you don't want it to run
RUN=1

# we have a special start: check it out!
START_ASM = staff-start.S

# these are all the tests.
PROGS := $(wildcard ./[1234567]-*.c)

PROGS := 2-brkpt-test.c  
PROGS := 1-watchpt-test.c  

# simplest: should run out of the box.
PROGS := 0-example-debug-id.c


CAN_EMIT=0
GREP_STR := 'TRACE:\|THREAD:\|ERROR:\|PANIC:' 
include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed
