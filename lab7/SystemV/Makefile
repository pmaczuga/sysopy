CC=gcc
LIBS=-lpthread -lrt

all: clean barber clients

barber:
	$(CC) -o barber common.c barber.c $(LIBS)

clients: clients.c common.c
	$(CC) -o clients common.c clients.c $(LIBS)

test:
	$(CC) -o test common.c test.c $(LIBS)

clean:
	rm -f test barber clients
