.PHONY:all
all:tcp_server tcp_client
tcp_client:tcp_client.c
	gcc -o $@ $^ -static
tcp_server:tcp_server.c -lpthread
	gcc -o $@ $^ 
.PHONY:clean
clean:
	rm -f tcp_client tcp_server
