CC      = gcc
CFLAGS  = -Wall -Wextra -g

all: city_manager monitor_report city_hub

city_manager:
	$(CC) $(CFLAGS) src/city_manager/*.c -o bin/city_manager

monitor_report:
	$(CC) $(CFLAGS) src/monitor_reports/*.c -o bin/monitor_reports

city_hub:
	$(CC) $(CFLAGS) src/city_hub/*.c -o bin/city_hub

clean:
	rm -f ./bin/city_manager ./bin/monitor_report ./bin/city_hub

