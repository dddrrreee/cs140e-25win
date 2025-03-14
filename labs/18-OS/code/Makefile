# "make checkoff" should run a bunch of tests.
#
# note: 
#  - these won't work with make check
#  - but they should pass their internal checks.
# PROGS = tests/4-cool-test.c
# PROGS = tests/3-stack-test.c
#
# PROGS = tests/2-no-stack-test.c
# 
# PROGS += tests/1-mov-test.c
# PROGS += tests/1-nop1-test.c
# PROGS += tests/1-nop10-test.c

# very simple tests. start here.
PROGS += tests/0-nop1-test.c
PROGS += tests/0-small1-test.c
PROGS += tests/0-nop10-test.c
PROGS += tests/0-mov-test.c

# this is fun to run.

PROGS = tests/4-cool-test.c
PROGS = tests/5-user-prog.c
PROGS := $(sort $(wildcard tests/[01234]-*test.c))

VM_DIR = ../../17-vm-page-table/code
PIN_DIR = ../../13-pinned-vm/code/

# INC += -I$(PIN_DIR)
INC += -I$(VM_DIR)

COMMON_SRC += $(VM_DIR)/mmu.c  
COMMON_SRC += pinned-vm.c  

STAFF_OBJS += $(PIN_DIR)/staff-pinned-vm.o  
STAFF_OBJS += $(VM_DIR)/staff-mmu-asm.o  
STAFF_OBJS += $(VM_DIR)/staff-mmu-except.o  

# we give you this code.  you can start tweaking it.
COMMON_SRC += eqx-os.c
COMMON_SRC += staff-full-except.c
COMMON_SRC += switchto-asm.S
COMMON_SRC += full-except-asm.S

STAFF_OBJS += ./staff-objs/staff-mmu.o  
STAFF_OBJS += ./staff-objs/staff-switchto-asm.o
STAFF_OBJS += ./staff-objs/staff-breakpoint.o
STAFF_OBJS += ./staff-objs/staff-full-except-asm.o


BOOTLOADER=my-install
RUN = 1
CAN_EMIT=1

OUR_START = staff-start.S

EXCLUDE ?= grep -v simple_boot
# GREP_STR := 'TRACE:\|THREAD:\|ERROR:\|PANIC:'

GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'

include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust-v2

all:: user-progs $(PROGS:.c=.bin)

user-progs/byte-array-0-hello.h: FORCE
	make -C user-progs

user-progs: FORCE
	make -C user-progs

clean::
	make -C user-progs clean
