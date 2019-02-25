CC=gcc
LIBS=-lpthread -lm

all: clean main filterCreator

main:
	$(CC) -o main main.c $(LIBS)

filterCreator:
	$(CC) -o filterCreator filterCreator.c $(LIBS)

clean:
	$(C) rm -f test main filterCreator
