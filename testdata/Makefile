CC = gcc
CFLAGS = -g
LDFLAGS = -g

all: clean parser

parser: parser.o 
	$(CC) $(LDFLAGS) -o $@ $^

parser.o: parser.c structures.h
	$(CC) $(CFLAGS) -c -Wall $<

clean:
	rm -f *~ *o *# parser
