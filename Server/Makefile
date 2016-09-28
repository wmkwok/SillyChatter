CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -g 

all: server_nonblocking

server_nonblocking: server_nonblocking.o registration.o
	$(CC) $(LDFLAGS) -o server_nonblocking server_nonblocking.o registration.o

registration.o: registration.c
	$(CC) $(CFLAGS) -c registration.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *.o *# server_nonblocking
