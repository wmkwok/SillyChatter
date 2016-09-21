CC = gcc
CFLAGS = -g
LDFLAGS = -g 

all: server_nonblocking

server_nonblocking: server_nonblocking.o registration.o
	$(CC) $(LDFLAGS) -o server_nonblocking server_nonblocking.o registration.o

%.o: %.c
	$(CC) $(LDFLAGS) -c $<

clean:
	rm -f *~ *.o *# server_nonblocking
