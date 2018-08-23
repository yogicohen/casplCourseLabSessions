all:client server

client: line_parser.o client.o
	gcc -g -Wall -o client line_parser.o client.o 

server: server.o common.o line_parser.o
	gcc -g -Wall -o server line_parser.o common.o server.o

line_parser.o: line_parser.c line_parser.h
	gcc -g -Wall -c -o line_parser.o line_parser.c

common.o: common.c
	gcc -g -Wall -c -o common.o common.c

client.o: client.c line_parser.h
	gcc -g -Wall -c -o client.o client.c

server.o: server.c common.h line_parser.h
	gcc -g -Wall -c -o server.o server.c

.PHONY: clean

clean: 
	rm -f *.o client
	rm -f *.o server
