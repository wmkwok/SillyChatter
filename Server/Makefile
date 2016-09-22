CC = gcc
CFLAGS = -g
LDFLAGS = -g 

all: server_nonblocking

server_nonblocking: server_nonblocking.o registration.o
	$(CC) $(LDFLAGS) -o server_nonblocking server_nonblocking.o registration.o

registration.o: registration.c
	$(CC) $(LDFLAGS) -c registration.c

%.o: %.c
	$(CC) $(LDFLAGS) -c $<

clean:
	rm -f *.txt *~ *.o *# server_nonblocking
