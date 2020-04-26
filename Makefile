all: psta

psta: client.c server.c
	gcc client.c -o client
	gcc server.c -o server -pthread
