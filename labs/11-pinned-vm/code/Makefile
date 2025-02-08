BOOTLOADER = my-install

PROGS += tests/1-test-basic.c
PROGS += tests/1-test-setup.c
PROGS += tests/1-test-two-addr.c
PROGS += tests/2-test-procmap.c

# your code will go here.
COMMON_SRC = pinned-vm.c

O = $(CS140E_2025_PATH)/libpi
STAFF_OBJS += $(O)/staff-objs/staff-full-except.o
STAFF_OBJS += $(O)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-switchto-asm.o
STAFF_OBJS += staff-mmu-asm.o  
STAFF_OBJS += staff-mmu-except.o  
STAFF_OBJS += staff-mmu.o  
STAFF_OBJS += staff-pinned-vm.o

# define this if you need to give the device for your pi
TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust

clean::
	make -C tests clean
