# start with this one if you have trouble.
TESTS = hello.bin
TESTS = $(wildcard ./*.bin)

# should not need this!
# TTYUSB = 

BOOT ?= my-install 
BOOTLOADER = $(BOOT) --trace-all

# we only look at this output
# GREP_STR := "TRACE:"
# GREP_STR := 'PI:\|TRACE:\|ERROR:\|PANIC:simple_boot'

# so we don't lose simple_boot
EXCLUDE := cat

include $(CS140E_2025_PATH)/libpi/mk/Makefile.check-v3

# run with the staff.
staff-check:
	make BOOT=pi-install check
