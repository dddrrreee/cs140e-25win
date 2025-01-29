PROGS = my-install.c
COMMON_SRC = put-code.c
CFLAGS += -I../2-pi-side

include $(CS140E_2025_PATH)/libunix/mk/Makefile.unix

trace:
	./my-install --trace-control hello.bin

trace-all:
	./my-install --trace-all hello.bin

.PHONEY: trace trace-all
