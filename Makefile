CC = gcc
CFLAGS = -Wall -Wextra

.PHONY: clean

server: http_server_with_poll.c
	$(CC) $(CFLAGS) -o server http_server_with_poll.c

performance_check: performance_test.sh
	./performance_test.sh > performance_logs.txt
	
benchmarking_check: benchmarking_test.sh
	./benchmarking_test.sh > benchmarking_logs.txt

clean:
	rm -f server performance_logs.txt benchmarking_logs.txt
