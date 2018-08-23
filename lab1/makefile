
# All Targets
all: wordCounter

# Executable "wordCounter" depends on the files wordCounter.o
wordCounter: wordCounter.o
	gcc -g -Wall -o wordCounter wordCounter.o

wordCounter.o: wordCounter.c
	gcc -g -Wall -c -o wordCounter.o wordCounter.c 

#tell make that "clean" is not a file name!
.PHONY: clean

#Clean the build directory
clean: 
	rm -f *.o wordCounter
