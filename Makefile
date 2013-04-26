CC=@gcc
CFLAGS=-Wall -Wextra -g -Wno-unused-parameter -Wno-sign-compare -Isrc -Itest -O3
LDFLAGS=-lm

SOURCES = $(wildcard test/*.c)
OBJECTS0= $(SOURCES:.c=)
OBJECTS = $(subst test,bin,$(OBJECTS0))


all:	thread.o $(OBJECTS)
	@echo " All done."

thread.o:	bin/thread.o

bin/thread.o:	src/thread.h src/thread.c
	@echo  -n " Compiling" $@ "... "
	$(CC) $(CFLAGS) -o bin/thread.o -c src/thread.c $(LDFLAGS)
	@echo "Done."



$(OBJECTS): $(SOURCES) bin/thread.o src/thread.c
	@echo -n " Compiling" $@ "... "
	$(CC) $(CFLAGS) -o $@ $(subst bin,test,$(@:%=%.c)) bin/thread.o $(LDFLAGS)
	@echo "Done."

bin/61-Voy_job: test/61-Voy_job.c
	@echo -n " ******Compiling" $@ "... "
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Done."

check:	thread.o $(OBJECTS)

	@echo " * * * Testing '01-main' * * *"
	@bin/01-main
	@echo ""

	@echo " * * * Testing '02-switch' * * *"
	@bin/02-switch
	@echo ""

	@echo " * * * Testing '11-join' * * *"
	@bin/11-join
	@echo ""

	@echo " * * * Testing '12-join-main' * * *"
	@bin/12-join-main
	@echo ""

	@echo " * * * Testing '21-create-many 42' * * *"
	@bin/21-create-many 42
	@echo ""

	@echo " * * * Testing '22-create-many-recursive 42' * * *"
	@bin/22-create-many-recursive 42
	@echo ""

	@echo " * * * Testing '31-switch-many 42 1337' * * *"
	@bin/31-switch-many 42 1337
	@echo ""

	@echo " * * * Testing '32-switch-many-join 42 1337' * * *"
	@bin/32-switch-many-join 42 1337
	@echo ""

	@echo " * * * Testing '51-fibonacci 10' * * *"
	@bin/51-fibonacci 10
	@echo ""

	@echo " * * * Testing '61-Voy_Commerce 8 1234' * * *"
	@bin/61-Voy_Commerce 8 1234
	@echo ""

	@echo " * * * Testing '71-Somme_Tableau 64' * * *"
	@bin/71-Somme_Tableau 64
	@echo ""

clean:
	@rm -f $(OBJECTS)
	@rm -f bin/*.o
	@echo " Cleaned."
