BOOTLOADER = my-install

PROGS := tests/1-test-basic-tutorial.c
PROGS += tests/1-test-one-addr.c
PROGS += tests/1-test-two-addr.c
PROGS += tests/2-test-lookup.c
PROGS += tests/2-test-procmap.c

# new test.
# uses PMU counters to check if caches were flushed, etc.
PROGS += tests/4-test-vm-cache-mgmt.c

# you'll have to copy this from lab 13.
COMMON_SRC += pinned-vm.c

# You'll modify these two files.
COMMON_SRC += your-mmu-asm.S
COMMON_SRC += mmu.c


O = $(CS140E_2025_PATH)/libpi
STAFF_OBJS += $(O)/staff-objs/staff-full-except.o
STAFF_OBJS += $(O)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-switchto-asm.o
STAFF_OBJS += $(O)/staff-objs/kmalloc.o

# you should be able to delete all of these after the lab.
STAFF_OBJS += staff-mmu-asm.o  
STAFF_OBJS += staff-mmu.o  

# these are just from lab 13: should be able to delete
# after you copy your code over.
STAFF_OBJS += staff-mmu-except.o  
STAFF_OBJS += staff-pinned-vm.o

# define this if you need to give the device for your pi
TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust

clean::
	make -C tests clean
