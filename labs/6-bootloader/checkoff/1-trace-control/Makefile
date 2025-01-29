# start with this one if you have trouble.
TESTS = hello.bin
TESTS = $(wildcard ./*.bin)

# should not need this!
# TTYUSB = 

BOOTLOADER = my-install --trace-control

# we only look at this output
GREP_STR := "TRACE:"
# so we don't lose simple_boot
EXCLUDE := cat

# include $(CS140E_2025_PATH)/libpi/mk/Makefile.check-v2
include $(CS140E_2025_PATH)/libpi/mk/Makefile.check-v3
