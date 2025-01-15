# show how to use wildcards to automate specifying everything
# that should get pulled into the makefile.
#
# the user defines the name of the executable in <NAME>.
#
# the <makefile> uses a wildcard pattern to grab all .c 
# files in the current directory and makes dependencies 
# simple by 
#   (1) using a second wildcard  to grab all .h's in the
#   current directory;
#   (2)  and making all source files depend on this list. 
#
# for a big project, this approach is overkill, but for a 
# small project gives you a quick-and-dirty + 
# trivial-to-understand <makefile>.
#
# note: this makefile is a generic template.  the only thing 
# you *would* need to change for a similarly constructed project 
# would be <NAME>
NAME=main
CFLAGS = -O -Wall

# get all headers: will make everything depend on them.  this
# is a downside in general, but for small stuff we don't care
# (next few modifications change this)
hdr = $(wildcard *.h)
src = $(wildcard *.c)
# get all source
obj = $(src:.c=.o)

# below will work as long as you define <obj>, <hdr> and <NAME>
all: $(NAME) test

$(NAME): $(obj) $(hdr)
	$(CC) $(CFLAGS) $(obj) -o $(NAME)

%.o: %.c $(hdr)
	$(CC) $(CFLAGS) -c $<  -o $@

###################################################################
# same as previous.

test: $(NAME) 
	./$(NAME) > out
	diff out $(NAME).ref
	@echo "makefile: program '$(NAME)' passed test"
	@rm -f out

# note this does assume all trash you want to delete
# is in the current directory.
clean:
	rm -f $(NAME) out *~ *.bak $(obj)

.PHONY: clean test all
