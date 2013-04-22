CC=@gcc
CFLAGS=-Wall -Wextra -g -Wno-unused-parameter -Wno-sign-compare
LDFLAGS=-lpthread

SOURCES = 01-main.c 02-switch.c 11-join.c 12-join-main.c 21-create-many.c 22-create-many-recursive.c 31-switch-many.c 32-switch-many-join.c 51-fibonacci.c
OBJECTS = $(SOURCES:.c=)



all:	thread.o $(OBJECTS)

thread.o:	../src/thread.h ../src/thread.c
	@echo  -n " Compiling " $@ "..."
	$(CC) $(CFLAGS) -o thread.o -c ../src/thread.c $(LDFLAGS)
	@echo "\t Done."

%:	%.c
	@echo  -n " Compiling " $@ "..."
	@gcc $(CFLAGS) -o $@ $^ thread.o $(LDFLAGS)
	@echo " \t Done."

check:	thread.o $(OBJECTS)

	@echo " * * * Testing '01-main' * * *";
	@./01-main
	@echo ""

	@echo " * * * Testing '02-switch' * * *";
	@./02-switch
	@echo ""

	@echo " * * * Testing '11-join' * * *";
	@./11-join
	@echo ""

	@echo " * * * Testing '12-join-main' * * *";
	@./12-join-main
	@echo ""

	@echo " * * * Testing '21-create-many 42' * * *";
	@./21-create-many 42
	@echo ""

	@echo " * * * Testing '22-create-many-recursive 42' * * *";
	@./22-create-many-recursive 42
	@echo ""

	@echo " * * * Testing '31-switch-many 42 1337' * * *";
	@./31-switch-many 42 1337
	@echo ""

	@echo " * * * Testing '32-switch-many-join 42 1337' * * *";
	@./32-switch-many-join 42 1337
	@echo ""

	@echo " * * * Testing '51-fibonacci 10' * * *";
	@./51-fibonacci 10
	@echo ""

clean:
	@rm -f $(OBJECTS)
	@rm -f *.o
	@echo " Cleaned."
