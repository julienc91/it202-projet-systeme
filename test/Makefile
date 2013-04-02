CC=gcc
LFLAGS=-Wall -Wextra

all:	contextes

contextes:	contextes.c
	$(CC) $(LFLAGS) -o $@ $^

clean:
	@rm contextes
	@rm -f *.o
	@echo "Cleaned."