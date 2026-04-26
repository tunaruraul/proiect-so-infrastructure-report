CC      = gcc
CFLAGS  = -Wall -Wextra -g
OBJS    = bin/main

compile:
	$(CC) $(CFLAGS) src/*.c -o $(OBJS)

run:
	./bin/main
