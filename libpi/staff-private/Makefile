# check that your path is defined.

include $(CS140E_2025_PATH)/libpi/defs.mk

# if you get source from other directories, add them to vpath too, ':' seperated
VPATH := .:

ifeq ($(USE_FP),1)
BUILD_DIR := ./objs-fp
TARGET := ../staff-objs/fp
else
EXTRA=make-fp
BUILD_DIR := ./objs
TARGET := ../staff-objs
endif


CFLAGS += -Wno-unused-function -Ilibc -DSTAFF_BUILD

# uart-buffer will buffer stuff.
# OBJS = gpio.o gpio-int.o uart.o gpio-pud.o kmalloc.o sw-uart.o  gpio-pud.o   interrupts-asm.o  spi.o interrupts-vec-asm.o  rpi-thread.o rpi-thread-asm.o sw-spi.o # backtrace.o # new-spi.o
OBJS = gpio.o uart.o gpio-pud.o kmalloc.o interrupts-asm.o 
# OBJS += gpio-int.o  
OBJS += staff-rpi-thread.o staff-rpi-thread-asm.o 
# OBJS += rpi-thread.o rpi-thread-asm.o 
# OBJS += interrupts-vec-asm.o

# staff-switchto-asm.o staff-full-except.o staff-full-except-asm.o sw-uart.o 


# sw-uart.o 
# spi.o i2c.o new-spi.o #  gpio-pud.o   interrupts-asm.o  spi.o interrupts-vec-asm.o  sw-spi.o # backtrace.o # new-spi.o


#interrupts-asm.o gpio-int.o uart-int.o thread-asm.o rpi-thread.o handle-swi-full.o spi.o mbox.o


# if you want to compile everything in a directory, do something like this.
#   SOURCES := $(wildcard ./*.c)
#   OBJS := $(patsubst %.c, %.o, $(SOURCES))

# this is putting the objects in the BUILD_DIR
OBJS := $(foreach o, $(OBJS), $(BUILD_DIR)/$(notdir $o))
# Get all depend files by replacing extensions
DEPS := $(OBJS:.o=.d)

CPP_ASFLAGS += -DSTAFF_BUILD

all: $(OBJS) copy  libpi  $(EXTRA)

$(EXTRA):
	make -C ./ USE_FP=1

copy:
	mkdir -p $(TARGET)
	cp $(BUILD_DIR)/*.o $(TARGET)

libpi:
	make -C ..

clean:
	rm -rf $(BUILD_DIR) $(TARGET)/*.o $(TARGET)/*/*.o objs/ objs-fp *~ tags *.o Makefile.bak  *.d

.PHONY: clean all tags  copy make-fp

#################################################################################
# the rest of this is dependency nonsense.   must be a cleaner way?
#

# XXX: We seem to need this rule to stop compilation with an error if the .c file
# has one: otherwise make does not stop compilation when generating .d.
# If compilation goes correctly, is not triggered.
$(BUILD_DIR)/%.o : %.c
$(BUILD_DIR)/%.o : %.c $(BUILD_DIR)/%.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<

# XXX: I think we need this to catch errors in .S as well.
$(BUILD_DIR)/%.o : %.S
$(BUILD_DIR)/%.o : %.S $(BUILD_DIR)/%.d
	$(CC) $(CPP_ASFLAGS) $<

# Rules to automatically generate dependencies and put in build directory
# We want to recompile everything if the makefile changes.
$(BUILD_DIR)/%.d: %.c Makefile
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -MT $@ -MMD -MP -MF $@ $< -o $(BUILD_DIR)/$(notdir $*).o

$(BUILD_DIR)/%.d: %.S Makefile
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPP_ASFLAGS) -c -MT $@ -MMD -MP -MF $@ $< -o $(BUILD_DIR)/$(notdir $*).o

# -MF  write the generated dependency rule to a file
# -MG  assume missing headers will be generated and don't stop with an error
# -MM  generate dependency rule for prerequisite, skipping system headers
# -MP  add phony target for each header to prevent errors when header is missing
# -MT  add a target to the generated dependency

# I don't get why we need this.  check the mad-make guy.
#   http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
$(BUILD_DIR)/%.d: ;
.PRECIOUS: $(BUILD_DIR)/%.d

# *unbelievable*: make clean doesn't skip this include even though the 
# target is after.
#   https://www.gnu.org/software/make/manual/html_node/Goals.html
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

