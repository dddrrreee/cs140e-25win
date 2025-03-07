BOOTLOADER = my-install

PROGS := tests/1-test-basic-tutorial.c
PROGS += tests/1-test-one-addr.c
PROGS += tests/1-test-two-addr.c
PROGS += tests/2-test-procmap.c
PROGS += tests/3-test-cache.c
PROGS += tests/4-test-dcache-inv.c  
PROGS += tests/4-test-vm-cache-mgmt.c

PROGS := $(wildcard tests/[1234]-*c)

# this is the one file you'll modify.
COMMON_SRC += pt-vm.c

STAFF_OBJS += staff-pt-vm.o

# useful helpers.
COMMON_SRC += mmu-helpers.c

# INC += -I./ -I$(YOUR_VM)/

################################################################
# if you want to drop in your stuff: should be able to flip the 
# definitions.

# these are just the startr-code trampolines from last time.
COMMON_SRC += your-mmu-asm.S
COMMON_SRC += mmu.c

YOUR_VM = $(CS140E_2025_PATH)/labs/15-vm-coherence/code.staff/
# you should be able to drop in your old stuff.
# COMMON_SRC += $(YOUR_VM)/mmu.c
# COMMON_SRC += $(YOUR_VM)/your-mmu-asm.S

# you should be able to delete all of these after the lab
# if you swap in yours
STAFF_OBJS += staff-mmu-asm.o  
STAFF_OBJS += staff-mmu.o  
# these are just from pinned v: should be able to delete
# if you swap yours in.
STAFF_OBJS += staff-mmu-except.o  

# same.
O = $(CS140E_2025_PATH)/libpi
STAFF_OBJS += $(O)/staff-objs/staff-full-except.o
STAFF_OBJS += $(O)/staff-objs/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-objs/staff-switchto-asm.o
STAFF_OBJS += $(O)/staff-objs/kmalloc.o

TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

GREP_STR := 'TRACE:\|HASH:\|ERROR:\|PANIC:\|SUCCESS:'
include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust-v2

clean::
	make -C tests clean

