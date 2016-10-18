CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -g 

all: server

server: server.o registration.o
	$(CC) $(LDFLAGS) -o server server.o registration.o

registration.o: registration.c
	$(CC) $(CFLAGS) -c registration.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *.o *# *.txt server
