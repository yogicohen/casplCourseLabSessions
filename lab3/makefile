all:task2c

task2c: main.o toy_printf.o
	gcc -g -Wall -o task2c main.o toy_printf.o

main.o: main.c
	gcc -g -Wall -c -o main.o main.c

toy_printf.o: toy_printf.c
	gcc -g -Wall -c -o toy_printf.o toy_printf.c

.PHONY: clean

clean: 
	rm -f *.o task2c
