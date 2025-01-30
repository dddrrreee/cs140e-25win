###############################################################
# uncomment this to build yours
# COMMON_SRC += uart.c

PROGS =  output-test-0-hello.c  
PROGS += output-test-1-hello-disable.c  
PROGS += output-test-3-emit.c

# swap these if your bootloader doesn't work.
BOOTLOADER = my-install
BOOTLOADER = pi-install

# set to 1 if you want to run.
RUN=0

# we use a custom cstart
COMMON_SRC += cstart.c 

L := $(CS140E_2025_PATH)/libpi/
STAFF_OBJS := $(L)/staff-objs/staff-sw-uart.o


# if doing checkoff use both uart and cstart.
ifdef CHECKOFF
    COMMON_SRC = uart.c cstart.c
    PROGS= $(wildcard output-test*.c)
endif

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed

checkoff:
	make -C ./ CHECKOFF=1 check
	@echo "checking that not using staff uart: if you see output is an error:"
	arm-none-eabi-nm $(BUILD_DIR)/*.elf  | grep staff_uart
