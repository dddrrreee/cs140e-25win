# makefile looks similar to 2-gpio/code/Makefile, but 
# we add:
#  1. the ability to # grab libpi.a and add it.
#  2. automatically run the binary.
PROG_SRC = hello.c 

MAKEFLAGS += --no-print-directory
MAKEFLAGS += --warn-undefined-variables

#############################################################
# get the standard definitions for different make variables.

ifndef CS140E_2025_PATH
$(error CS140E_2025_PATH is not set: this should contain the absolute path to where this directory is.  Define it in your shell's initialiation.  For example, .tcshrc for tcsh or .bashrc for bash)
endif

# defines LPI, MEMMAP, START, and CFLAGS 
# and all the tools.
include $(CS140E_2025_PATH)/libpi/defs.mk
#############################################################

# dependencies: recompile everything if any of these change.
DEPS := ./Makefile  $(LPP)/include/rpi.h

# we do start.o in a weird way: compile the libpi/staff-start.S
# into the current directory.  see the rule at the end of the
# makefile
objs := staff-start.o

# the name of the stripped binaries we make: these can be run
# using the bootloader.
progs   := $(PROG_SRC:.c=.bin)

# fake targets for each binary used to automatically run.
run_all    := $(PROG_SRC:.c=.run)

# disable built-in rules.
.SUFFIXES:

# our target: all the .bin files.
all:  $(progs) $(run_all)

# each fake target foo.run depends on foo.bin
$(run_all): %.run: %.bin
	pi-install ./$@

# always attempt to remake libpi.a
$(LPI):
	@make -C $(LPP) $(LPI)

# to cut down on rules have one rule to generate
#   - .elf:  the program
#   - .list: disassembled version of .elf so 
#     we can see what the machine code looks like
#  - .bin: stripped binary, run by pi-install
$(progs): %.bin: %.o $(objs)  $(DEPS)  $(MEMMAP)  $(LPI)
	$(LD) $(LDFLAGS) $(objs) $< -T $(MEMMAP) -o $*.elf  $(LPI)
	$(OD) -D  $*.elf > $*.list
	$(OCP) $*.elf -O binary $@

# how to generate .o's from the .c's.
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# we hardcode the start recipe to make things easier.
staff-start.o: $(START:.o=.S) $(DEPS)
	$(CC) -c $(CPP_ASFLAGS) $< -o $@

clean:
	rm -f *.bin *.list *.elf *.o Makefile.bak *~

.PHONY: all clean $(run_all)

# cheat and just keep all the .o's.
.PRECIOUS: %.o
