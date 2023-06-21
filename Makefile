CC = gcc
CFLAGS = -Wall

.PHONY: clean

server: http_server_with_poll.c index_html_gz.h http_parser/http_parser.h
	$(CC) $(CFLAGS) -o server http_server_with_poll.c http_parser/http_parser.c

index.html.gz: index.html
	gzip -9 < index.html > index.html.gz

index_html_gz.h: index.html.gz
	xxd -i index.html.gz > index_html_gz.h

performance_check: performance_test.sh
	./performance_test.sh > performance_logs.txt
	
benchmarking_check: benchmarking_test.sh
	./benchmarking_test.sh > benchmarking_logs.txt

clean:
	rm -f server performance_logs.txt benchmarking_logs.txt index_html_gz.h
