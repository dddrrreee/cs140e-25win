# will compare the programs
# stand-alone makefile for testing: aims to compare subsequent runs ("make check")
# against a previous reference run ("make emit")
#
#
# if you do:
#   - "make run" it will run all programs in TESTS
#   - "make emit" it emit .outs for all programs in TESTS
#   - "make check" it will check all programs in TESTS against their .out
#   - "make cksum" it will compute checksums for all programs in TESTS

# you can add a device path if you need it
BOOTLOAD = my-install

# list out the tests you want.
TESTS := $(wildcard ./[0123]-*.bin) $(wildcard ./prog-*.bin)


# we only look at this output
GREP_STR := "TRACE:"

.SUFFIXES: .out .run .check .emit

TEST_RUN   := $(patsubst %, %.run, $(TESTS))  
TEST_EMIT  := $(patsubst %, %.emit, $(TESTS))  
TEST_CHECK := $(patsubst %, %.check, $(TESTS))  
TEST_CKSUM := $(patsubst %, %.cksum, $(TESTS))  
TEST_OUT   := $(patsubst %.bin, %.out, $(TESTS))  

# set whatever you want the default action to be.
all: run

%.run: %
	@echo "\n-------------------[about to run <./$*]-----------------------";
	$(BOOTLOAD) ./$* 
	@echo "-------------------[done running <./$*>]------------------------------------";

# note: this will fail if output does not contain a trace.
# we pull everything into .out so that it's easier to debug.
%.emit: %
	$(BOOTLOAD) ./$* 2>&1 > /dev/null | grep $(GREP_STR) | grep -v 20200034 | uniq > ./$<.out

%.check: % 
	@echo -n "checking <$*>: ";
	@$(BOOTLOAD) ./$* 2>&1 > /dev/null | grep  $(GREP_STR) | grep -v 20200034 | uniq > ./$<.test
	@diff ./$<.out ./$<.test
	@echo " Success: <$*>!";
	@rm ./$*.test

# we could just cksum all the .outs, but this lets us be more targeted.
%.cksum: %.out
	cksum ./$*.out

# not sure if there is a better way: use these fake target to run our
# checks for everything.
# emit: $(TEST_EMIT) 
check: $(TEST_CHECK) 
run: $(TEST_RUN) 
cksum:  $(TEST_CKSUM)

clean:
	rm -f *~
    
cleanall:
	rm -f *.out

.PHONY: all clean run emit check cksum
