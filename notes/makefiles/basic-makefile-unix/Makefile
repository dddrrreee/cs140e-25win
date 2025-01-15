# example of how to run make in subdirectories and get results
SUBDIRS = 0-most-basic 
SUBDIRS += 1-incremental-compilation 
SUBDIRS += 2-pattern-rules 
SUBDIRS += 3-wildcards
SUBDIRS += 4-auto-dependency-gen
SUBDIRS += 5-safety

.PHONY: all check clean
all check clean: $(SUBDIRS)

all: TARGET=all
check: TARGET=check
clean: TARGET=clean

# No, you can't do TARGET=$@, or at least I don't know how to.

# recursive call to make
$(SUBDIRS): force
	$(MAKE) -s -C $@ $(TARGET)

clean:
	rm -f *~ *.bak

.PHONY: force
	force :;
