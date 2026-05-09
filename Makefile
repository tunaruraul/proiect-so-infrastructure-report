CC      = gcc
CFLAGS  = -Wall -Wextra -g
OBJS    = bin/city_manager

compile:
	$(CC) $(CFLAGS) src/*.c -o $(OBJS)

clean:
	rm -f ./bin/city_manager

run:
	./bin/main --role manager --user raul view 2
