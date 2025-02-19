# "make checkoff" should run a bunch of tests.
#
# note: 
#  - the 4,3,2 tests won't work with make check
#  - but they should pass their internal checks.
# PROGS = tests/4-cool-test.c
# PROGS = tests/3-stack-test.c
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

# we start with this b/c it's cute.
PROGS = tests/4-cool-test.c

# this is fun to run.
# PROGS = tests/4-cool-test.c

# we give you these files code.  you can start tweaking them
# when pass the tests.
COMMON_SRC += eqx-threads.c
COMMON_SRC += staff-full-except.c

# this calls staff: replace routines 1 by 1.
COMMON_SRC += switchto-asm.S

# this calls staff: replace routines 1 by 1.
COMMON_SRC += full-except-asm.S

# you'll have to just drop in your breakpoint to replace this.
# copy it over here.
STAFF_OBJS += staff-breakpoint.o

# we give you a kmalloc
STAFF_OBJS += $(CS140E_2025_PATH)/libpi/staff-objs/kmalloc.o

# as you replace routines comment these out.
STAFF_OBJS += staff-full-except-asm.o
STAFF_OBJS += staff-switchto-asm.o

BOOTLOADER=my-install
RUN = 1
CAN_EMIT=1

# look in here for the stackless tests.
OUR_START = staff-start.S

EXCLUDE ?= grep -v simple_boot
# GREP_STR := 'TRACE:\|THREAD:\|ERROR:\|PANIC:'

GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'

include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust

# START_ASM = staff-start.S
# include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed

checkoff:
	make -f ./checkoff.mk
