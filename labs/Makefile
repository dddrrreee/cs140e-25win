# Makefile to build or clean all labs.
SUBDIRS += 0-pi-setup 
SUBDIRS += 1-trusting-trust 
SUBDIRS += 2-gpio
SUBDIRS += 3-cross-checking
#SUBDIRS += 5-interrupts
#SUBDIRS += 6-threads
#SUBDIRS += 7-bootloader
#SUBDIRS += 8-uart
#SUBDIRS += 9-device-int
#SUBDIRS += 10-debug-hw
#SUBDIRS += 11-imu-i2c
## SUBDIRS += 12-i2c+mailboxes
#SUBDIRS += 13-ss-equiv
#SUBDIRS += 14-ss-equiv-deux
#SUBDIRS += 15-pinned-vm

.PHONY: all check clean
all check clean: $(SUBDIRS)

all: TARGET=all
check: TARGET=check
clean: TARGET=clean

# No, you can't do TARGET=$@, or at least I don't know how to.

# recursive call to make
$(SUBDIRS): force
	$(MAKE) -s -C $@ $(TARGET) RUN=0

clean:
	rm -f *~ *.bak

.PHONY: force
	force :;
