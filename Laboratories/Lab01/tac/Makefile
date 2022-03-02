CC=gcc
CFLAGS=-Wall -Wextra -Werror

build: tac

tac: tac.o
	$(CC) $(CFLAGS) $^ -o $@

tac.o: tac.c
	$(CC) $(CFLAGS) -c $^ -o $@

run:
	./tac test

clean:
	rm *.o tac
