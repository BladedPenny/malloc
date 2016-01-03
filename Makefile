CC=gcc
CFLAGS=-Wall -g
LDFLAGS=

OBJS=heap209.o diagnostics.o

all: test-basic1

test-basic1: test-basic1.o $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
test-c4patela: test-c4patela.c $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
tests: test-basic1 test-c4patela
	./test-basic1
	./test-c4patela
%.o: %.c heap209.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f test-basic1 *.o
