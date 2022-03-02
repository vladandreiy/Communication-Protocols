CC=g++
CFLAGS=-I.

all: client

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) -o client client.cpp requests.cpp helpers.cpp buffer.cpp -g -Wall

run: client
	./client

clean:
	rm -f *.o client
