# real problem with previous makefile:
#  1. had to specify dependencies manually.
#  2. makes it hard to use wildcard rules accurately: either 
#     you specify everything depends on everything or you
#     have a bunch of rules, defeating the purpose.
# 
# instead we use gcc to automatically generate dependencies using
# the -MMD family of flags.
# 
NAME=main


# standard dependency flags so gcc will generate the includes
# that a file depends on.  "$*.d" places the .d file in the same
# location as the .c
# from 'make' maintainer:
#   https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
DEPFLAGS = -MT $@ -MMD -MP -MF $*.d
CFLAGS = -O -Wall $(DEPFLAGS)
CC = gcc
# doesn't make sense to generate dependency files during linking.
# so we have a different set of flags (empty for now)
LDFLAGS = 

# in general use the ":=" assignment b/c it will evaluate
# the right hand side (RHS) once, at the assignment.
src := $(wildcard *.c)
obj := $(src:.c=.o)
# .d files for dependencies
dep := $(src:.c=.d)

all: $(NAME) test

$(NAME): $(obj) $(dep)
	$(CC) $(LDFLAGS) $(obj) -o $(NAME)

# note: we add each file's .d as a dependency to catch 
# if (1) the .d got deleted or (2) the .d was put in a 
# different location. not having this guardrail has burned
# use in the past.
%.o: %.c  %.d
	$(CC) $(CFLAGS) -c $<  -o $@

# include the generated dependency rules.
# open up a .d file in your editor to see 
# why this works!
# 
# put it last so the rules can't mess with ours.
-include $(dep)

# tell gcc not to worry if a .d doesn't exist since it gets
# made during compilation. 
$(dep): %.d: ;

# print out the different variables.  we do it using
# comments b/c it's weird, but you could also use
# "echo" or "printf"
print:
	# src = <$(src)>
	# obj = <$(obj)>
	# dep = <$(dep)>
	# DEPFLAGS = <$(DEPFLAGS)>
	
######################################################################
# ~ same as previous makefiles but we also delete .d files

test: $(NAME) 
	./$(NAME) > out
	diff out $(NAME).ref
	@echo "makefile: program '$(NAME)' passed test"
	@rm -f out

clean:
	rm -f $(NAME) out *~ *.bak $(obj) $(dep)

.PHONY: clean test all print
