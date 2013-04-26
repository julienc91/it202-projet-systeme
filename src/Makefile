CC=gcc
LFLAGS=-Wall -Wextra -g -lpthread -D_FORTIFY_SOURCE=2

all: thread example contextes

contextes: contextes.c
	$(CC) -O0  -o $@ $^ $(LFLAGS)

thread:	thread.c
	$(CC) -O0  -c $^ $(LFLAGS)

example: thread.c example.c
	$(CC) -O0 $^ -o $@ $(LFLAGS)

clean:
	@rm contextes example
	@rm -f *.o
	@echo "Cleaned."
