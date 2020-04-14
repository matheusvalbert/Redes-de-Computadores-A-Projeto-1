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
#include <sys/types.h>
#include <dirent.h>

struct infocliente
{
	struct sockaddr_in client;
	int ns;
};

pthread_mutex_t mutex;
void INThandler(int);
int s;

void *tratamento(void *informacoes);
void listar(int);
void receber();
void enviar();

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

void *tratamento(void *informacoes) {

	struct sockaddr_in client;
	struct infocliente info;
	int ns;
	info = *(struct infocliente*) informacoes;
	client = info.client;
	ns = info.ns;
	char comando[15];

	printf("conexao aceita!\n");

	do {

		receberMensagem(ns, comando, sizeof(comando));

		if(strcmp(comando, "listar") == 0)
			listar(ns);
		else if(strcmp(comando, "receber") == 0)
			receber();
		else if(strcmp(comando, "enviar") == 0)
			enviar();

	}while(strcmp(comando, "encerrar") != 0);

	close(ns);
	pthread_exit(NULL);
}

void listar(int ns) {
	size_t tamanho = 101;
	char buff[100], file_names[200], *cwd;	
	int filesize;
	DIR *dir;
	struct dirent *dp;
	file_names[0] = '\0';

	printf("listar\n");

	cwd = getcwd(buff,tamanho);
	if ((dir = opendir (cwd)) == NULL) 
	{
        	perror ("Cannot open current directory!");
        	exit (1);
   	}
	while ((dp = readdir (dir)) != NULL) 
	{
		if ( !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") )
		{
     			// nao mostrar o diretorio atual e o anterior
		}
		else
		{
			if(file_names[0] == '\0')
				strcpy(file_names,dp->d_name);
			else
			{
				filesize = strlen(file_names);
				file_names[filesize] = ' ';
				file_names[filesize + 1] = '\0';
				strcat(file_names,dp->d_name);
			}
			
		}
	}
	filesize = strlen(file_names);
	file_names[filesize] = '\n';
	file_names[filesize + 1] = '\0';
	enviarMensagem(ns, file_names, strlen(file_names));
	closedir(dir);
}

void receber() {

	printf("receber\n");
}

void enviar() {

	printf("enviar\n");
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
