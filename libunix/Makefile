# you can modify this file: we won't be touching it the rest of the quarter.

# you can override these if you want.
CC ?= gcc

# the list of sources to compile --- can list them out too, but this is easy.
LIB_SRC := $(wildcard ./*.c)
LIBNAME = libunix.a

# all the include locations.
INC +=  -I$(CS140E_2025_PATH)/libunix

# define any flags you want to pass to CC
CFLAGS = -g -Og -Wall -Wno-unused-function -Werror -Wno-unused-variable

# methods used to make everything.
# include staff-rules.mk
include mk/Makefile.lib.template
