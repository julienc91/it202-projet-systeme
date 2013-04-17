CC=gcc
LFLAGS=-Wall -Wextra -g -lpthread

all: thread example contextes

contextes: contextes.c
	$(CC)  -o $@ $^ $(LFLAGS)

thread:	thread.c
	$(CC)  -c $^ $(LFLAGS)

example: thread.c example.c
	$(CC)  $^ -o $@ $(LFLAGS)

clean:
	@rm contextes example
	@rm -f *.o
	@echo "Cleaned."
