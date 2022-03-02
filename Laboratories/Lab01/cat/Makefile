CC=gcc
CFLAGS=-Wall -Wextra -Werror

build: cat

cat: cat.o
	$(CC) $(CFLAGS) $^ -o $@

cat.o: cat.c
	$(CC) $(CFLAGS) -c $^ -o $@

run:
	./cat test

clean:
	rm *.o cat
