SUBDIRS=  1-sw-uart-put8 2-uart 3-fake-pi
SUBDIRS =  2-uart 0-cycles-per-sec

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
