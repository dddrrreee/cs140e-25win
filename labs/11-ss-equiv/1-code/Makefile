# change this if you want!
BOOTLOADER=my-install

# part 3
# you'll have to create these.  they should
# have the same .out's as part 2, other than
# the cpsr will differ.
#   PROGS += 3-reg-save.c
#   PROGS += 3-reg-save-full.c

# part 2
#PROGS += 2-reg-save-full.c
#PROGS += 2-reg-save.c

# part 1
#PROGS += 1-rfe-example.c
#PROGS += 1-rfe-blk.c
#PROGS += 1-rfe-switchto-user.c

# part 0
#PROGS  += 0-any-mode-sp-lr.c
#PROGS  += 0-mem-user-sp-lr.c

# start with this one!
PROGS  += 0-cps-user-sp-lr.c

# we use a custom start b/c we have routines at fixed
# locations.
START_ASM = staff-start.S

# these are the files used in the library.
# should be one file probably.  doesn't really
# matter where you put your assembly as long 
# as you remember where it is.
COMMON_SRC += 0-user-sp-lr-asm.S  
COMMON_SRC += 1-rfe-asm.S  
COMMON_SRC += 2-reg-save-asm.S

# automatically run.
# RUN=1

# CAN_EMIT=1
LIB_POST += $(CS140E_2025_PATH)/lib/libgcc.a

# GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed
