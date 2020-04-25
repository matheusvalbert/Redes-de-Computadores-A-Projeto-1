#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>
#include "tcp.h"

struct infocliente
{
	struct sockaddr_in client;
	int ns;
};

pthread_mutex_t mutex;
void INThandler(int);
int s;

void *tratamento(void *informacoes);
void listar(int ns, char ip[], int p);
void receber(int ns, char ip[], int p);
void enviar(int ns, char ip[], int p);

int main(int argc, char **argv) {

	pthread_t tratarClientes;
	unsigned short port;
	int ns, namelen, tc;
	struct sockaddr_in client;
	struct sockaddr_in server;
	struct infocliente informacoes;
	char arg[50];
	signal(SIGINT, INThandler);

	if (argc != 2)  {

		fprintf(stderr, "Use: %s porta\n", argv[0]);
		exit(1);
	}

	if (pthread_mutex_init(&mutex, NULL) != 0)  {

		printf("falha iniciacao semaforo\n");
		exit(1);
	}

	strcpy(arg, argv[1]);

	iniciaConexaoServer(&s, &port, &server, &client, &namelen, arg);

	while(1) {

		aceitaConexao(&ns, &s, &client, &namelen);

		informacoes.ns = ns;
		informacoes.client = client;

		tc = pthread_create(&tratarClientes, NULL, tratamento, &informacoes);
    	if (tc) {
			printf("ERRO: impossivel criar um thread consumidor\n");
			exit(-1);
    	}

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

	printf("conexao aceita - IP: %s - Porta: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

	while(1) {

		receberMensagem(ns, comando, sizeof(comando));

		if(strcmp(comando, "listar") == 0)
			listar(ns, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		else if(strcmp(comando, "receber") == 0)
			receber(ns, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		else if(strcmp(comando, "enviar") == 0)
			enviar(ns, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		else if(strcmp(comando, "encerrar") == 0) {

			close(ns);
			printf("encerrar - IP: %s - Porta: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
			pthread_exit(NULL);
		}
	}
}

void listar(int ns, char ip[], int p) {
	size_t tamanho = 101;
	char buff[100], file_names[200], *cwd;
	int filesize;
	DIR *dir;
	struct dirent *dp;
	file_names[0] = '\0';

	printf("listar- IP: %s - Porta: %d\n", ip, p);

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
	printf("%s/n", file_names);
	filesize = strlen(file_names);
	file_names[filesize] = '\n';
	file_names[filesize + 1] = '\0';
	filesize++;
	enviarMensagem(ns, &filesize, sizeof(int));
	enviarMensagem(ns, file_names, strlen(file_names));
	closedir(dir);
}

void receber(int ns, char ip[], int p) {

	unsigned short port;
	struct hostent *hostnm;
    struct sockaddr_in server;
	int sData;
	char argument[50];

	int porta;
	receberMensagem(ns, &porta, sizeof(porta));
	sprintf(argument, "%d", porta);

	char host[50];
	receberMensagem(ns, host, sizeof(host));

	iniciaConexaoClient(host, argument, &port, hostnm, &server);
	socketConectar(&sData, &server);


	char arg[50];
	printf("receber - IP: %s - Porta: %d\n", ip, p);
	int argTam;
	receberMensagem(ns, &argTam, sizeof(argTam));
	receberMensagem(ns, arg, argTam);
	int size;
	unsigned char buffer[1024];

	FILE *ptr;
	ptr = fopen(arg,"rb");

	fseek(ptr, 0, SEEK_END);
	size = ftell(ptr);
	fseek(ptr, 0, SEEK_SET);

	enviarMensagem(ns, &size, sizeof(size));

	int nvezes = size/1024;

	while(nvezes != 0) {

		fread(buffer,1024,1,ptr);
		enviarMensagem(sData, buffer, 1024*sizeof(char));
		nvezes--;
	}

	if(size%1024 != 0) {

		fread(buffer,size%1024,1,ptr);
		enviarMensagem(sData, buffer, (size%1024)*sizeof(char));
	}

	fclose(ptr);
	close(sData);
}

void enviar(int ns, char ip[], int p) {

	unsigned short port;
	struct hostent *hostnm;
    struct sockaddr_in server;
	int sData;
	char argument[50];

	int porta;
	receberMensagem(ns, &porta, sizeof(porta));
	sprintf(argument, "%d", porta);

	char host[50];
	receberMensagem(ns, host, sizeof(host));

	iniciaConexaoClient(host, argument, &port, hostnm, &server);
	socketConectar(&sData, &server);

	char arg[50];
	unsigned char buffer[1024];
	int size, tamanho;
	printf("enviar - IP: %s - Porta: %d\n", ip, p);
	int argTam;
	receberMensagem(ns, &argTam, sizeof(argTam));
	receberMensagem(ns, arg, argTam);

	receberMensagem(ns, &size, sizeof(size));

	FILE *ptr;

	ptr = fopen(arg,"wb");

	int nvezes = size/1024;

	while(nvezes != 0) {

		receberMensagem(sData, buffer, 1024*sizeof(char));
		fwrite(buffer,1024,1,ptr);
		nvezes--;
	}

	if(size%1024 != 0) {

		receberMensagem(sData, buffer, (size%1024)*sizeof(char));
		fwrite(buffer,size%1024,1,ptr);
	}

	fclose(ptr);
	close(sData);
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
