#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "thread.h"
#include "ftp.h"

#define clrscr() printf("\e[1;1H\e[2J")

void printMenu();
void receberComando(char comando[], char arg1[], char arg2[]);
bool conectar(int *s, char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server);
void listar(int s);
void receber(int s, char arg1[], char arg2[]);
void enviar(int s, char arg1[], char arg2[]);
void encerrar(int s);

int main() {

	unsigned short port;
	struct hostent *hostnm;
    struct sockaddr_in server;
	bool flag = false;
	int s;

	char comando[50], arg1[150], arg2[50];

	while(1) {

	printMenu();
	receberComando(comando, arg1, arg2);

	if(strcmp(comando, "conectar") == 0 && arg1 != NULL && arg2 != NULL)
		flag = conectar(&s, arg1, arg2, &port, hostnm, &server);
	else if(strcmp(comando, "listar\n") == 0 && flag == true)
		listar(s);
	else if(strcmp(comando, "receber") == 0 && arg1 != NULL && arg2 != NULL && flag == true)
		receber(s, arg1, arg2);
	else if(strcmp(comando, "enviar") == 0 && arg1 != NULL && arg2 != NULL && flag == true)
		enviar(s, arg1, arg2);
	else if(strcmp(comando, "encerrar\n") == 0 && flag == true)
		{
			encerrar(s);
			flag = false;
		}
	else if(flag == false)
		printf("Conexao ainda nao realizada!\n");
	else
		printf("comando invalido!");

	}

	return 0;
}

void printMenu() {

	//clrscr();
	printf("Opcoes:\n");
	printf("conectar <nome do servidor> <porta do servidor>\n");
	printf("listar\n");
	printf("receber <arquivo remoto> <arquivo local>\n");
	printf("enviar <arquivo local> <arquivo remoto>\n");
	printf("encerrar\n");
	printf("> "); 
}

void receberComando(char comando[], char arg1[], char arg2[]) {

	char comandoCompleto[250], *aux;

	fgets(comandoCompleto, sizeof(comandoCompleto), stdin);
	
	aux = strtok(comandoCompleto, " ");
	if(aux != 0)
		strcpy(comando, aux);

	aux = strtok(NULL, " ");
	if(aux != NULL)
		strcpy(arg1, aux);

	aux = strtok(NULL, " ");
	if(aux != NULL) {

		strcpy(arg2, aux);
		arg2[strlen(arg2) - 1] = '\0';
	}
}

bool conectar(int *s, char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server) {

	iniciaConexaoClient(arg1, arg2, port, hostnm, server);
	socketConectar(s, server);
	return true;
}

void listar(int s) {
	char file_names[200];
	enviarMensagem(s, "listar", sizeof("listar"));
	receberMensagem(s, file_names, sizeof(file_names));
	printf("%s", file_names);
}

void receber(int s, char arg1[], char arg2[]) {

	enviarMensagem(s, "receber", sizeof("receber"));

	unsigned short port;
	struct hostent *hostnm;
    struct sockaddr_in server;
	int sData;
	char argument[50];

	int porta;
	receberMensagem(s, &porta, sizeof(porta));
	sprintf(argument, "%d", porta);

	char host[50];
	receberMensagem(s, host, sizeof(host));
	printf("%s\n", host);

	iniciaConexaoClient(host, argument, &port, hostnm, &server);
	socketConectar(&sData, &server);

	int argTam = strlen(arg1);
	enviarMensagem(s, &argTam, sizeof(argTam));
	enviarMensagem(s, arg1, strlen(arg1));
	unsigned char *buffer;
	int size, tamanho;
	
	receberMensagem(s, &size, sizeof(size));

	buffer = malloc(size);

	receberMensagem(sData, buffer, size*sizeof(char));

	FILE *ptr;

	ptr = fopen(arg2,"wb");

	fwrite(buffer,size,1,ptr);

	fclose(ptr);

	free(buffer);

	close(sData);
}

void enviar(int s, char arg1[], char arg2[]) {

	enviarMensagem(s, "enviar", sizeof("enviar"));

	unsigned short port;
	struct hostent *hostnm;
    struct sockaddr_in server;
	int sData;
	char argument[50];

	int porta;
	receberMensagem(s, &porta, sizeof(porta));
	sprintf(argument, "%d", porta);

	char host[50];
	receberMensagem(s, host, sizeof(host));
	printf("%s\n", host);

	iniciaConexaoClient(host, argument, &port, hostnm, &server);
	socketConectar(&sData, &server);

	int argTam = strlen(arg2);
	enviarMensagem(s, &argTam, sizeof(argTam));
	enviarMensagem(s, arg2, strlen(arg2));
	int size;
	unsigned char *buffer;
	
	FILE *ptr;
	ptr = fopen(arg1,"rb");

	fseek(ptr, 0, SEEK_END);
	size = ftell(ptr);
	fseek(ptr, 0, SEEK_SET);

	enviarMensagem(s, &size, sizeof(size));

	buffer = malloc(size);

	printf("%d\n", size);

	fread(buffer,size,1,ptr);

	enviarMensagem(sData, buffer, size*sizeof(char));

	fclose(ptr);

	free(buffer);

	close(sData);
}

void encerrar(int s) {
	enviarMensagem(s, "encerrar", sizeof("encerrar"));
	close(s);
}
