CC = gcc
CFLAGS = -g
LDFLAGS = -g

all: client server_nonblocking

client: client.o registration.o
	$(CC) $(LDFLAGS) -o $@ $^

server_nonblocking: server_nonblocking.o
	$(CC) $(LDFLAGS) -o server_nonblocking server_nonblocking.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *o *#
