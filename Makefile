all:
	gcc test.c -o test
	gcc server.c -o server
	gcc client.c -o client
clean:
	rm test server client
	rm -r tom
	rm -r frank
