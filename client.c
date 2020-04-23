#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "tcp.h"

#define clrscr() printf("\e[1;1H\e[2J")

void printMenu();
void receberComando(char comando[], char arg1[], char arg2[]);
bool conectar(int *s, char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server);
void listar(int s);
void receber(int s, char arg1[], char arg2[]);
void enviar(int s, char arg1[], char arg2[]);
void encerrar(int s);

int cout = 50;
char host[50];

int main() {

	clrscr();
	printMenu();

	unsigned short port;
	struct hostent *hostnm;
    struct sockaddr_in server;
	bool flag = false;
	int s;

	char comando[50], arg1[50], arg2[50];

	while(1) {

	printf("> ");
	receberComando(comando, arg1, arg2);

	if(strcmp(comando, "conectar") == 0 && arg1 != NULL)
		flag = conectar(&s, arg1, arg2, &port, hostnm, &server);
	else if(strcmp(comando, "listar\n") == 0 && flag == true)
		listar(s);
	else if(strcmp(comando, "receber") == 0 && arg1 != NULL && flag == true)
		receber(s, arg1, arg2);
	else if(strcmp(comando, "enviar") == 0 && arg1 != NULL && flag == true)
		enviar(s, arg1, arg2);
	else if(strcmp(comando, "encerrar\n") == 0 && flag == true)
		{
			encerrar(s);
			flag = false;
			printf("conexao encerrada\n");
		}
	else if(flag == false)
		printf("Conexao ainda nao realizada!\n");
	else
		printf("comando invalido!\n");

	}

	return 0;
}

void printMenu() {

	printf("Opcoes:\n");
	printf("conectar <nome do servidor> <porta do servidor>\n");
	printf("listar\n");
	printf("receber <arquivo remoto> <arquivo local>\n");
	printf("enviar <arquivo local> <arquivo remoto>\n");
	printf("encerrar\n");
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
	else if(strcmp(comando, "conectar") == 0) {

		strcpy(arg2, "5000");
		arg1[strlen(arg1) - 1] = '\0';
	}
	else {

		arg1[strlen(arg1) - 1] = '\0';
		strcpy(arg2, arg1);
	}
}

bool conectar(int *s, char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server) {

	iniciaConexaoClient(arg1, arg2, port, hostnm, server);
	socketConectar(s, server);
	strcpy(host, arg1);
	printf("conexao aceita\n");
	return true;
}

void listar(int s) {
	char file_names[200];
	enviarMensagem(s, "listar", sizeof("listar"));
	int tamanho;
	receberMensagem(s, &tamanho, sizeof(tamanho));
	receberMensagem(s, file_names, tamanho);
	printf("arquivos:\n-");
	for(int i = 0; i < tamanho; i++)
		if(file_names[i] == '\0')
			break;
		else if(file_names[i] == ' ')
			printf("\n-");
		else
			printf("%c", file_names[i]);
}

void receber(int s, char arg1[], char arg2[]) {

	enviarMensagem(s, "receber", sizeof("receber"));

	unsigned short port; 
	int nsData, namelen, sData;
	struct sockaddr_in client;
	struct sockaddr_in server;
	char argument[50];

	int porta = 5000 + cout;
	sprintf(argument, "%d", porta);
	enviarMensagem(s, &porta, sizeof(porta));
	cout++;

	iniciaConexaoServer(&sData, &port, &server, &client, &namelen, argument);

	enviarMensagem(s, host, sizeof(host));

	aceitaConexao(&nsData, &sData, &client, &namelen);

	int argTam = strlen(arg1);
	enviarMensagem(s, &argTam, sizeof(argTam));
	enviarMensagem(s, arg1, strlen(arg1));
	unsigned char buffer[1024];
	int size, tamanho;
	
	receberMensagem(s, &size, sizeof(size));

	FILE *ptr;

	ptr = fopen(arg2,"wb");

	int nvezes = size/1024;

	while(nvezes != 0) {

		receberMensagem(nsData, buffer, 1024*sizeof(char));
		fwrite(buffer,1024,1,ptr);
		nvezes--;
	}

	if(size%1024 != 0) {

		receberMensagem(nsData, buffer, (size%1024)*sizeof(char));
		fwrite(buffer,size%1024,1,ptr);
	}

	fclose(ptr);
	close(nsData);
	close(sData);

	printf("arquivo recebido - Nome no servidor: %s - Nome no cliente: %s\n", arg1, arg2);
}

void enviar(int s, char arg1[], char arg2[]) {

	enviarMensagem(s, "enviar", sizeof("enviar"));

	unsigned short port; 
	int nsData, namelen, sData;
	struct sockaddr_in client;
	struct sockaddr_in server;
	char argument[50];

	int porta = 5000 + cout;
	sprintf(argument, "%d", porta);
	enviarMensagem(s, &porta, sizeof(porta));
	cout++;

	iniciaConexaoServer(&sData, &port, &server, &client, &namelen, argument);

	enviarMensagem(s, host, sizeof(host));

	aceitaConexao(&nsData, &sData, &client, &namelen);

	int argTam = strlen(arg2);
	enviarMensagem(s, &argTam, sizeof(argTam));
	enviarMensagem(s, arg2, strlen(arg2));
	int size;
	unsigned char buffer[1024];
	
	FILE *ptr;
	ptr = fopen(arg1,"rb");

	fseek(ptr, 0, SEEK_END);
	size = ftell(ptr);
	fseek(ptr, 0, SEEK_SET);

	enviarMensagem(s, &size, sizeof(size));

	int nvezes = size/1024;

	while(nvezes != 0) {

		fread(buffer,1024,1,ptr);
		enviarMensagem(nsData, buffer, 1024*sizeof(char));
		nvezes--;
	}

	if(size%1024 != 0) {

		fread(buffer,size%1024,1,ptr);
		enviarMensagem(nsData, buffer, (size%1024)*sizeof(char));
	}

	fclose(ptr);
	close(nsData);
	close(sData);

	printf("arquivo enviado - Nome no cliente: %s - Nome no servidor: %s\n", arg1, arg2);
}

void encerrar(int s) {
	enviarMensagem(s, "encerrar", sizeof("encerrar"));
	close(s);
}
