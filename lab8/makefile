all:myELF

myELF: task2.o
	gcc -m64 -g -Wall -o myELF task2.o

task2.o: task2.c
	gcc -m64 -g -Wall -c -o task2.o task2.c

.PHONY: clean

clean: 
	rm -f *.o myELF
