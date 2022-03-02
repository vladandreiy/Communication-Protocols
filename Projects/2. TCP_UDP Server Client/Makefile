CC = g++
CFLAGS = -Wall -g
DEPS = helpers.h 
OBJ = server.o subscriber.o

all: server subscriber

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

server: server.o
	$(CC) -o $@ $< $(CFLAGS)

subscriber: subscriber.o
	$(CC) -o $@ $< $(CLAFGS)

.PHONY: clean run_server run_subscriber


clean:
	rm -rf server subscriber $(OBJ)
