CC = gcc
CFLAGS = -g
LDFLAGS = -g

all: clean client

client: client_server.o commands.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *~ *o *# client
