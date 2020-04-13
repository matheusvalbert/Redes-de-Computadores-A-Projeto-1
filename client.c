#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "thread.h"
#include "ftp.h"

#define clrscr() printf("\e[1;1H\e[2J")

void printMenu();
void receberComando(char comando[], char arg1[], char arg2[]);
bool conectar(int *s, char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server);
void listar(int s);
void receber(int s);
void enviar(int s);
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
		receber(s);
	else if(strcmp(comando, "enviar") == 0 && arg1 != NULL && arg2 != NULL && flag == true)
		enviar(s);
	else if(strcmp(comando, "encerrar\n") == 0 && flag == true)
		encerrar(s);
	else
		printf("comando invalido!");

	}

	return 0;
}

void printMenu() {

	clrscr();
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

	aux = strtok(0, " ");
	if(aux != 0)
		strcpy(arg1, aux);

	aux = strtok(0, " ");
	if(aux != 0)
		strcpy(arg2, aux);
}

bool conectar(int *s, char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server) {

	iniciaConexaoClient(arg1, arg2, port, hostnm, server);
	socketConectar(s, server);
	return true;
}

void listar(int s) {

	enviarMensagem(s, "listar", sizeof("listar"));
}

void receber(int s) {

	enviarMensagem(s, "receber", sizeof("receber"));
}

void enviar(int s) {

	enviarMensagem(s, "enviar", sizeof("enviar"));
}

void encerrar(int s) {

	enviarMensagem(s, "encerrar", sizeof("encerrar"));
	close(s);
}
