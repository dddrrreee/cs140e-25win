# good breakdown: http://cs107e.github.io/guides/make/
ODFLAG = -d 
ODFLAG = -d  -S 
ODFLAG = -d 
CFLAGS = -O3 -g -Wall -Werror -std=gnu99
PROGS = 2-wait.o wait-not-full.o  0-timer.o 1-timer.o 1-timer-fix.o 2-wait.o 3-fb.o 4-fb.o 1-timer-fix.real.o 5-barrier.o 5-fb.o

all : $(PROGS)

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c $^ -o $@
	arm-none-eabi-objdump $(ODFLAG) $@ >  $@.dis
	cat $@.dis

clean:
	rm -f  *.dis *.o *~ *.s
