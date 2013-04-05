CC=gcc
LFLAGS=-Wall -Wextra

all:	contextes thread

contextes:	contextes.c
	$(CC) $(LFLAGS) -o $@ $^

thread:	thread.c
	$(CC) $(LFLAGS) -c $^

clean:
	@rm contextes
	@rm -f *.o
	@echo "Cleaned."
