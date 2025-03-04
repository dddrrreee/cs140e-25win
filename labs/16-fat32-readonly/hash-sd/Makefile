PROGS = hash-files.c
COMMON_SRC = 

# CS140E_USE_STAFF = 1
include $(CS140E_2025_PATH)/libunix/mk/Makefile.unix

all:: hash

hash:
	./hash-files $(CS140E_2025_PATH)/firmware/*

.PHONEY: hash
