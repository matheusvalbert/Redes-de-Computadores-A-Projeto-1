all: ftp

ftp: ftp.o thread.o client.o server.o
	gcc ftp.o thread.o client.o -o client
	gcc ftp.o thread.o server.o -o server
	rm -rf *.o
	rm -rf *.h.gch

client.o: client.c
	gcc -c client.c

server.o: server.c
	gcc -c server.c

thread.o: thread.c
	gcc -c thread.c

ftp.o: ftp.c
	gcc -c ftp.c
