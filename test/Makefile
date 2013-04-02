CC=gcc
LFLAGS=#-Wall -Wextra

SOURCES = 01-main.c 02-switch.c 11-join.c 12-join-main.c 21-create-many.c 22-create-many-recursive.c 31-switch-many.c 32-switch-many-join.c 51-fibonacci.c
OBJECTS = $(SOURCES:.c=)



all:	$(OBJECTS)

thread.o:	thread.h thread.c
	$(CC) $(LFLAGS) -o thread.o -c thread.c

%:	%.c thread.o
	$(CC) $(LFLAGS) -o $@ $^ thread.o

check:	$(OBJECTS)

	@for i in $(OBJECTS) ; do \
	echo " ";\
        echo " *** Testing '" $$i "'"; \
	./$$i ; \
	done


clean:
	@rm -f $(OBJECTS)
	@rm -f *.o
	@echo "Cleaned."