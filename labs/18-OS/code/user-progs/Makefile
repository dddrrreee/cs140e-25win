# the tests in decreasing order of difficulty.
PROGS := 0-hello.c 1-fork.c 0-printk-hello.c 1-fork-waitpid.c

# the headers we compile them to.
PROG_HEADERS := $(patsubst %.c, byte-array-%.h, $(PROGS))

COMMON_SRC := libos-cstart.c 
CFLAGS += -I..
START = libos-asm.o

TTYUSB = 
BOOTLOADER = my-install
RUN = 1

MEMMAP := small-proc.ld
RUN = 0

CODE_GEN = ./code-gen-src/code-gen

include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-v3

all:: $(CODE_GEN) $(PROGS:.c=.bin) $(PROG_HEADERS)

byte-array-%.h: %.bin $(CODE_GEN)
	$(CODE_GEN)  $< > byte-array-$*.h

# add more actions to clean
clean::
	rm -f *.o $(PROG_HEADERS)
	make -C code-gen-src clean

$(CODE_GEN):  $(CODE_GEN).c
	make -C code-gen-src
