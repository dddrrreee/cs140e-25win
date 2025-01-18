# this is the final, basic UNIX makefile.  you should 
# be able to use it for most simple projects.  it's 
# main limitation is that it puts all the generated
# files (.o and .d) in the same directory as their
# associated source file.  this makes the logic simple,
# but from an aesthetic standpoint might annoy you.
# the fancy makefiles we build side-step this limitation,
# albeit with more complexity and navigating a few
# nasty pitfalls.
# 
# we extend the previous makefile to show that you 
# can trivially automatically grab files from other 
# directories.
#
# we add some safety features to the previous Makefile.  
# it means there's a bit more code, it's all code that 
# you just do once and then reuse in all subsequent makefiles.
#
# 1. disable built-in rules so we don't get any surprises.
# 2. explicitly specify the list of files that a rule 
#    can generate.
# 3. make compilation depend on the makefile itself: if
#    it changes, remake.
# 4. make clean if the makefile ever changes.

################################################################
# 1. compute all files that we generate and use.

NAME=main

# can easily add other wildcards if you want.
src := $(wildcard *.c) $(wildcard ./*/*.c)
obj := $(src:.c=.o)
dep := $(src:.c=.d)

# -I. grabs header files from the various directories.
CFLAGS = -I. $(INC) -Og -Wall -MT $@ -MMD -MP -MF $*.d
CC := gcc

# automatically compute -I include flags based on
# source directories (you could specify manually) 
# to handle the case where files include headers
# from other directories.
src_dirs :=  $(sort $(dir $(src)))
CFLAGS += $(foreach d, $(src_dirs), -I$(d))
LDFLAGS = 

# remove all "~" and ".bak" files in all source dirs
# easy to add other files (e.g., macos seems to puke 
# everywhere)
trash    := $(foreach d, $(src_dirs), $(wildcard $d/*~))
trash    += $(foreach d, $(src_dirs), $(wildcard $d/*.bak))
trash    := $(sort $(trash))

###############################################################
# 2. safety / some config.

# error checking: make sure <NAME> is just a name, not a .c file.
$(if $(findstring .c, $(NAME)), \
        $(error "`$(NAME) is a .c file, not a generic name!"))

# disable built-in rules for all files.
.SUFFIXES: 

# don't print when change directories.
MAKEFLAGS += --no-print-directory

# this is part of a hack to automatically run 
# "make clean" whenever the makefile gets modified.
#
# pull out every Makefile from the confusingly-named
# <MAKEFILE_LIST> (which can contain .d files if they
# get included)
MK_LIST := $(filter-out %.d, $(MAKEFILE_LIST))
# timestamp file used as comparison.
TIME_STAMP := ./.timestamp

# added as dependency to makefile rules.
DEP := $(TIME_STAMP) $(MK_LIST)

##############################################################
# 3. build rules.

all: $(NAME) test

# overkill method to run "make clean" if any makefile has changed.
$(TIME_STAMP): $(MK_LIST)
	@echo "A makefiles <$(MK_LIST)> has changed: going to run 'make clean'"
	@make clean
	@mkdir -p $(dir $@)
	@touch $@

$(NAME): $(obj) $(dep) 
	$(CC) $(LDFLAGS) $(obj) -o $(NAME)

# delete built-in rules for .c and .d
%.o:%.c
%.d:%.c

# explicitly state the files this rule generates.
$(obj): %.o: %.c  %.d $(DEP)
	$(CC) $(CFLAGS) -c $<  -o $@

-include $(dep)

# tell gcc not to worry if a .d doesn't exist since it gets
# made during compilation. 
$(dep): %.d: ;

test: $(NAME) 
	./$(NAME) > out
	diff out $(NAME).ref
	@echo "makefile: program '$(NAME)' passed test"
	@rm -f out

clean:
	rm -f $(NAME) out $(trash) $(obj) $(dep) $(TIME_STAMP)
 
.PHONY: clean test all print
