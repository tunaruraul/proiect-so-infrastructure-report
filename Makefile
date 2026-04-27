CC      = gcc
CFLAGS  = -Wall -Wextra -g
OBJS    = bin/main

compile:
	$(CC) $(CFLAGS) src/*.c -o $(OBJS)

clean:
	rm -f ./bin/main

run:
	./bin/main --role manager --user raul view 2
