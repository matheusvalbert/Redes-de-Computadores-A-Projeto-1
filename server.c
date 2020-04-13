#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "thread.h"
#include "ftp.h"

struct infocliente
{
	struct sockaddr_in client;
	int ns;
};

pthread_mutex_t mutex;
void INThandler(int);
int s;

void *tratamento(void *informacoes) {

	struct sockaddr_in client;
	struct infocliente info;
	int ns;
	info = *(struct infocliente*) informacoes;
	client = info.client;
	ns = info.ns;
	printf("conexao aceita!\n");
}

int main(int argc, char **argv) {

	pthread_t tratarClientes;
	unsigned short port; 
	int ns, namelen, tc;
	struct sockaddr_in client;
	struct sockaddr_in server;
	struct infocliente informacoes;
	char arg[10];
	signal(SIGINT, INThandler);

	if (argc != 2)  {

		fprintf(stderr, "Use: %s porta\n", argv[0]);
		exit(1);
	}

	iniciarMutex(&mutex);

	strcpy(arg, argv[1]);

	iniciaConexaoServer(&s, &port, &server, &client, &namelen, arg);

	while(1) {

		aceitaConexao(&ns, &s, &client, &namelen);

		informacoes.ns = ns;
		informacoes.client = client;
		
		tc = criarThread(tratarClientes, tratamento, &informacoes);

		usleep(250);
	}

	return 0;
}

void  INThandler(int sig)
{	
	int i = 0;
	void *ret;
	pthread_mutex_destroy(&mutex);
	close(s);
	pthread_exit(NULL);
	exit(0);
}
