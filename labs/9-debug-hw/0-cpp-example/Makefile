all:
	gcc -E macro.c > macro.i
	indent macro.i
	cat macro.i

clean:
	rm -f macro.i *~
