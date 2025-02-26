# easiest
# PROGS := 0-no-ack-check.c
# then.
# PROGS := 0-ack-check.c

# ordered easy to harder
PROGS += $(wildcard tests/0-*.c)
PROGS += $(wildcard tests/1-*.c)
PROGS += $(wildcard tests/2-*.c)
PROGS += $(wildcard tests/3-*.c)

# PROGS := tests/2-one-way-ack-16bytes.c

# PROGS := $(wildcard tests/[0123]-*.c)

LIBS += $(CS140E_2025_PATH)/lib/libgcc.a

COMMON_SRC := nrf-hw-support.c nrf-public.c nrf-driver.c

STAFF_OBJS += staff-nrf-driver.o

STAFF_OBJS += $(CS140E_2025_PATH)/libpi/staff-objs/staff-hw-spi.o

# uncomment if you want it to automatically run.
RUN=1

BOOTLOADER = my-install
EXCLUDE ?= grep -v simple_boot
GREP_STR := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust

checkoff:
	make -f ./Makefile PROGS="$(wildcard tests/[0123]-*.c)" check
	# make -f ./Makefile PROGS="$(ALL)" check
