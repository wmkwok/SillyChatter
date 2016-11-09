CC = gcc
CFLAGS = -g
LDFLAGS = -g

all: clean parser

parser: parser.o 
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *o *# parser
