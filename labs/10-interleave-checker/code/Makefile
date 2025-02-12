# work through all the tests in tests/ in order.  you
# should look at the tests!!
#
# if you build the code needed for yield:
#   PROGS := $(wildcard tests-2.0/[023567]*-test.c)

PROGS := tests/4-trylock-ok-test.c
PROGS := tests/3-circular-ok-test.c
PROGS := tests/1-epsilon-should-fail-test.c
PROGS := tests/2-increment-fail-test.c

# everything [ignore 1]
PROGS := $(wildcard tests/[023567]*-test.c)

# simple vacuous test.
PROGS := tests/0-epsilon-test.c

# all your code will be in here.
COMMON_SRC += check-interleave.c   

# trivial asm to invoke system calls
COMMON_SRC += syscall-invoke-asm.S

# you can drop your versions in from 140e.
STAFF_OBJS += staff-full-except-asm.o
STAFF_OBJS += staff-full-except.o
STAFF_OBJS += staff-switchto-asm.o
STAFF_OBJS += staff-single-step.o

# switch to pi-install if you want to use the staff.
BOOTLOADER=my-install
# runs the code automatically.  set to 0 if not.
RUN = 1

# lets you emit .out files.
# CAN_EMIT=1

EXCLUDE ?= grep -v simple_boot
GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'
include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust
# include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed
