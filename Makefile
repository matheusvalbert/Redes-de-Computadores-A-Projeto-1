all: psta

psta: tcp.o client.o server.o
	gcc tcp.o client.o -o client
	gcc tcp.o server.o -o server -pthread

client.o: client.c
	gcc -c client.c

server.o: server.c
	gcc -c server.c

tcp.o: tcp.c
	gcc -c tcp.c
