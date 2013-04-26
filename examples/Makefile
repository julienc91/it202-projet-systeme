CC=gcc
CFLAGS=-Wall -Wextra -g -Wno-unused-parameter
LDFLAGS=

all: example contextes

thread.o: ../src/thread.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

contextes: contextes.c thread.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

example: example.c thread.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	@rm contextes example
	@rm -f *.o
	@echo "Cleaned."
