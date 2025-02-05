# uncomment if you want to use staff libpi as a quick check.
# LPI = ../staff-objs/libpi.a

# include staff for everything so can run completely.
PROGS := 0-test-checks.c 1-test-run.c
COMMON_SRC := interrupt-asm.S

# uncomment if you want it to automatically run.
RUN=1

# change if you don't want this one.
# BOOTLOADER = pi-install
BOOTLOADER = my-install

# we use a higher optimization level for timings.
OPT_LEVEL = -O2

EXCLUDE ?= grep -v simple_boot
include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed

checkoff:
	make -f ./checkoff.mk
