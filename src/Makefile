CC=gcc
LFLAGS=-Wall -Wextra -g -lpthread

all: thread example contextes

contextes: contextes.c
	$(CC) $(LFLAGS) -o $@ $^

thread:	thread.c
	$(CC) $(LFLAGS) -c $^

example: thread.c example.c
	$(CC) $(LFLAGS) $^ -o $@

clean:
	@rm contextes example
	@rm -f *.o
	@echo "Cleaned."
