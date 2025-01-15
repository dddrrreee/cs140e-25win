# This makefile a more robust makefile that handles relative paths.
# 
# 1. throws all .o's in <BUILD_DIR>.
# 2. uses eval to generate the explicit rule for each .c->.o 
#    so that (1) VPATH doesn't screw us and (2) so we 
#    handle relative paths.
# 3. as before explicitly lists the .d file that each .o depends
#    on so it gets remade if the .d is deleted or doesn't exist
#    b/c of the wrong name.
# 4. all compilation rules depend on the makefile itself so will
#    conservatively rerun if the makefile changes.
#

# the one program we make.  you could also do this automatically
# if the files that contain main() have some naming convention.
# or if you seperate out the main() files from the rest.
NAME := main 

# delete all implicit rules, removing one of the big causes of make
# screwing things up.  in addition, this also removes
# weird errors when making non-existant targets (e.g., if you 
# have a rule for "foo" and type "make fo" instead of "make foo").
.SUFFIXES:

# if you want to do all .c files in this directory and 
# every subdirectory.
# SRC  :=  $(wildcard ./*.c) $(wildcard ./*/*.c)

# we explicitly list out the files we compile --- we use this to 
# check that `make` won't grab any synonym file not listed.  (e.g,
# that ./bar/foo.c doesn't get picked up instead of foo/foo.c)
SRC  :=  ./a.c ./main.c ./b.c ./foo/foo.c ./bar/bar.c ../relative-dir/baz.c ./no-include.c
# uncomment this (so we get a duplicate file) and look at the warnings!
# SRC += ./foo.c

# should add checks for bad BUILD_DIR's (e.g. ./)
BUILD_DIR = ./objs

# put all .o and .d for each .c source file in <BUILD_DIR>
reloc := $(foreach o, $(SRC), $(BUILD_DIR)/$(notdir $o))
objs := $(reloc:.c=.o)
deps := $(reloc:.c=.d)

# automatically make a list of all "*~" and "*.bak" trash files 
# in all the different source directories so `make clean` 
# can delete them.
dirs := $(sort $(dir $(SRC)))
trash := $(foreach d, $(dirs), $(wildcard $d/*~ $d/*.bak))

# add whatever flags you want to pass to the compiler.
CFLAGS := -Og -I. -Wall -Werror  
CFLAGS += -Wno-pointer-sign -Wno-unused-function -Wno-unused-variable

# XXX: unclear if we should have this seperate from the rule below.
#
# standard dependency flags so gcc will generate the includes
# that a file depends on.  "$*.d" places the .d file in the same
# location as the .c
# from 'make' maintainer:
#   https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) 

# pull out every Makefile from the confusingly-named
# <MAKEFILE_LIST> (which can contain .d files if they
# get included)
# 
# we add this as a dependency on each compile rules, so 
# that they will rerun if the Makefile ever changes.  
# eliminates some issues with changing compiler
# options.  To be even more hygenic we could do the hack of 
# running `make clean`
DEPS := $(filter-out %.d, $(MAKEFILE_LIST))

all: $(NAME)

# if we get an error for duplicate or undefined symbols it could 
# be just that or it could be b/c there are duplicate files.
$(NAME): $(objs) $(deps) $(DEPS)
	@echo "$(COMPILE.c) $(objs) -o $(NAME)"
	@$(COMPILE.c) $(objs) -o $(NAME) \
        || (echo "\n**This error could be b/c you have duplicate files that have collided: did make emit any warnings aboud recipes?**\n"; exit 1)

# remove built in rule for .c->.o and .c->.d (should be 
# redundant b/ we removed all .SUFFIXES)
%.o : %.c
%.d : %.c

###########################################################################
# the new trick: eval.
#   https://www.gnu.org/software/make/manual/html_node/Eval-Function.html
#
# we use eval to explicitly generate the makefile rules
# for each .o/.d so that we have complete
# control over which .c each .o/.d is generated from.
#
define COMPILE_RULE
$(BUILD_DIR)/$(notdir $(1:.c=.o)): $(1) $(BUILD_DIR)/$(notdir $(1:.c=.d)) $(DEPS)
	@mkdir -p $(BUILD_DIR)
	$(COMPILE.c) -c $$< -o $$@ -MT $$@ -MMD -MP -MF $$(@:.o=.d)
endef

# Apply the compilation rule for each source file
$(foreach src,$(SRC),$(eval $(call COMPILE_RULE,$(src))))

# catch if there are any other .o's that slipped through somehow.
%.o: 
	$(error "Trying to make a .o ($@) that isn't part of our source list")

###########################################################################

# tell gcc not to worry if a .d doesn't exist since it gets
# made during compilation. 
$(deps): %.d: ;

# we include these last so the <deps> rules don't mess w/ the rules
# above.
include $(deps)

# remove objs/deps and any trash files, along with 
# the BUILD_DIR
clean:
	rm -f $(deps) $(objs) $(trash) $(NAME)
	rm -rf $(BUILD_DIR)

# just for debugging: there's no gdb for make, so we
# resort to printf's :).  
print:
	@echo "SRC=<$(SRC)>"
	@echo "deps=<$(deps)>"
	@echo "objs=<$(objs)>"
	@echo "trash=<$(trash)>"
	@echo "dirs=<$(dirs)>"

.PHONY: all clean print
