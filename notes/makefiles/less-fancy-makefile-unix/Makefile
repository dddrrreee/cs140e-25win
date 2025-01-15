###########################################################
# Below follows my current view of the "safest" approach to 
# compiling a big multi-language (C, asm) project that spans
# directories and putting the results in a <BUILD_DIR>
#
# Limits:
#   1. does not handle relative paths that go outside the
#      makefile directory.
#
#      See: `fancy-makefile-unix`  for how to handle this 
#      problem.  likely a better approach overall.
#
#   2. Long: but this is mostly b/c of comments and b/c it
#      shows other tricks (that you can simply delete).
#
# Given:
#   1. <PROGS>: a list of programs to generate.  These are 
#      standalone program files with a `notmain`.
#   2. <C_SRC>: list of all shared C source code linked with 
#      all <p> \in <PROGS>.
#   3. <ASM_SRC>: list of all shared .S assembly code linked 
#      with all <p> \in <PROGS>
#   4. <PREBUILT_OBJS>: prebuilt .o object files (e.g., from staff).
#
# Then:
#   1. automatically generate a <.bin> and <.list> for all <M>
#   2. put all compiler results (.o, .d, .elf) in <BUILD_DIR>
#
# The Makefile should remake exactly what is needed no matter
# what you do.  In particular, if you 
#   - 'touch'/modify any .c, .S, .h file or the <Makefile> 
#     itself.
#   - delete any .o, .d, .list, .elf, .bin file
# it should remake the affected and dependent pieces.
#
# Specific decisions to increase safety:
# 0. use gcc to auto-gen dependencies [no surprise: common]
# 1. key: no <VPATH>.  we eliminate the need for <VPATH> and its
#    many betrayals by putting every <.o> and <.d> in a isomophically 
#    mirrored directory in <BUILD_DIR>.  this lets us use 
#    pattern rules to match without using <VPATH>.
#    e.g.  
#        foo/bar/baz.c  
#    will have:
#        <BUILD_DIR>/foo/bar/baz.o
#        <BUILD_DIR>/foo/bar/baz.d
#    This burns time, but for our class is fast enough it doesn't
#    matter.  We've had at least two very nasty bugs from VPATH
#    so it's good to not have +1 more.
# 2. Every rule explicitly specifies the <.d> it depends on. 
#    This catches if there is a mistake in where you think 
#    a file's <.d> is or if it got deleted.  [safety net for 
#    common mistake.  I made this mistake in an old 140e makefile 
#    and `make` silently ran without complaint, happily using stale
#    inputs]
# 3. We disable default rules (%.o:%.c, %.o:%.S), and use static 
#    targets to eliminate unexpected behavior.  Also to catch 
#    if any file slips through our rules.
# 4. The use of static targets also eliminates the need for 
#    .PRECIOUS directives which don't safely handle control-C 
#    during compilation and appear to have some other issues
#    (which I admit I didn't investigate).
# 5. As a small change, we explicitly do a `mkdir -p <BUILD_DIR>`
#    for every rule that needs it.  this is wasteful of CPU cycles
#    but (hopefully) falls in the "obvious correct result" category.
#    In the past I've used "| $(BUILD_DIR)" order dependencies, but 
#    they seem to sometimes have issues even with the use of the <FORCE>
#    hack.  
# 6. We always attempt to rebuild libpi/libpi.a rather than doing anything
#    fancy.  Again: waste cycles so we don't have to have a logical 
#    argument as to why we don't need to.
#
# NOTE: other than <PREBUILT_OBJS> all files must be in the current 
# directory or its children. It is illegal to be higher in the directory 
# structure.  (We don't check this)

PROGS = main.c
# all your common C files: note that unlike w VPATH there's no
# danger in having multiple files with same name (e.g., foo.c)
C_SRC = foo/foo.c bar/bar.c a.c  foo.c
# all your common asm files.
ASM_SRC := asm.S

# all prebuilt .o's
PREBUILT_OBJS := prebuilt-b.o

# what directory to put the generated files; you can change this.
BUILD_DIR = ./objs

# If you want to be especially careful, set this flag to 1.
# The Makefile will automatically run `make clean` whenever 
# if it or any included Makefile changes.  
CLEAN_IF_MAKEFILE_CHANGES := 1

###############################################################
# Config and sanity checks

# pulls in 140e definitions of common variables:
#   - <LPP>, <LPI>, <START>, <MEMMAP>, <CFLAGS>
#   - tool names: <CC>, <OD>, etc
include $(CS140E_2024_PATH)/libpi/defs.mk

# don't print when change directories.
MAKEFLAGS += --no-print-directory
# warn when we use unset variables; useful when developing
# MAKEFLAGS += --warn-undefined-variables

ifndef PROGS
    $(error "<PROGS> not is defined.")
endif

# Nasty if you make this mistake!
ifeq ($(BUILD_DIR),.)
    $(error "<BUILD_DIR> cannot be '.' or clean will remove everything.")
endif
# Ditto.
ifeq ($(BUILD_DIR),)
    $(error "<BUILD_DIR> cannot be empty.")
endif

###############################################################
# make all the different target names.

# put all .o from every source in <BUILD_DIR>
prog_objs := $(patsubst %.c, $(BUILD_DIR)/%.o, $(PROGS))
csrc_objs := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRC))
asm_objs  := $(patsubst %.S, $(BUILD_DIR)/%.o, $(ASM_SRC))

# every .o has a .d dependency file.
all_objs := $(prog_objs) $(csrc_objs) $(asm_objs)
all_deps := $(all_objs:.o=.d)

# flags for generating .d
DEPFLAGS =  -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.d

# all bin and list files: these are not in the <BUILD_DIR>
all_bins := $(PROGS:.c=.bin)
all_list := $(PROGS:.c=.list)
# all elf: these are in <BUILD_DIR>
all_elf  := $(prog_objs:.o=.elf)

# pull out all makefiles from the confusingly named
# <MAKEFILE_LIST> (which can contain .d files)
MK_LIST := $(filter-out %.d, $(MAKEFILE_LIST))

TIME_STAMP := $(BUILD_DIR)/.timestamp
ifneq ($(CLEAN_IF_MAKEFILE_CHANGES),0)
    DEPS    := $(TIME_STAMP)
endif

# make everything depend on the makefile itself.
DEPS      += $(MK_LIST)
# extra linker dependencies
LINK_DEPS := $(LPI) $(START) $(MEMMAP) $(DEPS)

###############################################################
# compilation rules: this is where all the tricky stuff
# is.

all: $(all_bins)

# overkill method to run "make clean" if any makefile has changed.
# we *could* just recompile, however i think there can be cases
# where this won't be enough.
$(TIME_STAMP): $(MK_LIST)
	@echo "A makefiles <$(MK_LIST)> has changed: going to run 'make clean'"
	@make clean
	@mkdir -p $(dir $@)
	@touch $@

# we use the <FORCE> hack to force make to 
# always try recompiling libpi.a
$(LPP)/libpi.a: FORCE
	@make -C $(LPP) 

# a fake target that forces a rule to always run.
.PHONY: FORCE
FORCE:

# rule to generate all <BUILD_DIR>/.elf files.
link_objs := $(asm_objs) $(csrc_objs) 
link_deps := $(link_objs:.o=.d)
link_objs += $(PREBUILT_OBJS)
$(all_elf): %.elf: %.o %.d $(link_objs) $(link_deps) $(LINK_DEPS)
	@mkdir -p $(dir $@)
	$(LD) $(START) $< $(link_objs) -T $(MEMMAP) -o $@ $(LPI)

# every .elf gets disassembled into a .list.  use `-D` if
# you want to see the C source.
$(all_list): %.list: $(BUILD_DIR)/%.elf
	$(OD) -d $< > $@

# make a .bin from every .elf.
$(all_bins): %.bin: $(BUILD_DIR)/%.elf %.list $(DEPS)
	$(OCP) $< -O binary $@

# delete built-in rules for .c and .d
%.o:%.c
%.d:%.c
# all C object files use this rule.
c_objs := $(csrc_objs) $(prog_objs)
$(c_objs): $(BUILD_DIR)/%.o: %.c $(BUILD_DIR)/%.d  $(DEPS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# delete built in rules for .S
%.d:%.S
%.o:%.S

# all .S asm files use this rule: note the explicit .d 
# dependency.
$(asm_objs): $(BUILD_DIR)/%.o: %.S $(BUILD_DIR)/%.d $(DEPS)  
	@mkdir -p $(dir $@)
	$(CC) $(CPP_ASFLAGS) $(DEPFLAGS) -c $< -o $@

# tell gcc not to worry if a .d doesn't exist since it gets
# made during compilation.   you can test that this rule
# doesn't compromise dependencies by using touching or deleting
# any .d
$(all_deps): %.d: ;

# include this after the compilation rules
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(all_deps)  # include dep files
endif
endif


###############################################################
# example of computing ctags across a bunch of different places.
#   make tags
# 
# macos complains about non-unique names so we do some sort/uniq
# stuff.
#
# if you don't want to tag libpi, just comment out:
libpi_src  +=  $(sort $(wildcard $(LPP)/*.[cS] $(LPP)/*/*.[cS]))

# find all the .c .S and .h files we depend on.
# all .c and .S in this project
all_src  := $(PROGS) $(C_SRC) $(ASM_SRC) $(libpi_src)
# now find all .h in every directory
src_dirs := $(sort $(dir $(all_src)))
all_hdrs += $(foreach d, $(src_dirs), $(wildcard $d/*.h))
# unique set of files so macos ctags does not complain.
all_tags := $(sort $(all_src) $(all_hdrs))

tags: 
	@ctags $(all_tags)

###############################################################
# common support rules.  nothing fancy about them.
#

# we just nuke the entire $(BUILD_DIR) so don't put 
# stuff there!
clean:
	rm -rf $(BUILD_DIR)  *~ *.bak $(all_bins) $(all_list) tags

# remove all "~" and ".bak" files in all source dirs
# easy to add other files (e.g., macos seems to puke 
# everywhere)
all_src  := $(PROGS) $(C_SRC) $(ASM_SRC)
src_dirs := $(sort $(dir $(all_src)))
trash    := $(foreach d, $(src_dirs), $(wildcard $d/*~))
trash    += $(foreach d, $(src_dirs), $(wildcard $d/*.bak))
trash    := $(sort $(trash))
deep-clean:
	@echo "Deleting trash: <$(trash)> and <$(BUILD_DIR)>"
	@rm -f $(trash)
	@make -s -C ./  clean

# all targets that do not generate a file with the target
# name.
.PHONY: clean all print tags deep-clean

# print useful variables to help see what is going on.
#
# the commands:
#   make -d
#   make --debug
# also give useful information.
print:
	@echo "----------------- user defined -----------------";
	@echo "PROGS        = <$(PROGS)>"
	@echo "C_SRC        = <$(C_SRC)>"
	@echo "ASM_SRC      = <$(ASM_SRC)>"
	@echo "PREBUILT_OBJS= <$(PREBUILT_OBJS)>"
	@echo "----------------- computed -----------------";
	@echo "prog_objs    =<$(prog_objs)>"
	@echo "csrc_objs    =<$(csrc_objs)>"
	@echo "asm_objs     =<$(asm_objs)>"
	@echo "c_objs       =<$(c_objs)>"
	@echo "all_deps     =<$(all_deps)>"
	@echo "link_objs    =<$(link_objs)>"
	@echo "link_deps    =<$(link_deps)>"
	@echo "all_tags    =<$(all_tags)>"
	@echo "----------------- aux vars -----------------";
	@echo "DEPS         =<$(DEPS)>"
	@echo "LINK_DEPS    =<$(LINK_DEPS)>"
	@echo "MK_LIST      =$(MK_LIST)"
