# simple definitions for making libpi.a
#  - you should only ever have to modify SRC or STAFF_OBJS

# to add your gpio.c:
#   1. copy gpio.c to libpi/src/gpio.c
#   2. comment out:
STAFF_OBJS  +=  ./staff-objs/gpio.o
#   3. uncomment:
# SRC += src/gpio.c

# if you want to use your own pullup-puldown
# 1. put it in src/gpio-pud.c)
# 2. add it to SRC
# 3. comment out ours below:
STAFF_OBJS  +=  ./staff-objs/gpio-pud.o

STAFF_OBJS  +=  ./staff-objs/uart.o
STAFF_OBJS  +=  ./staff-objs/gpio-int.o 

# these are all the locations that get made into
# libpi.a
#
# you should not have to modify this variable.  
SRC :=  $(SRC)                              \
        $(wildcard ./libc/*.[Sc])           \
        $(wildcard ./staff-src/*.[Sc])      \
        staff-start.S

# hack to minimize git conflicts: we do various customizations
# in there; but probably would be clearer to inline it.
include ./manifest.mk
