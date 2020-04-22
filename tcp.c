#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

void iniciaConexaoClient(char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server) {

	hostnm = gethostbyname(arg1);
	if (hostnm == (struct hostent *) 0)  {

		fprintf(stderr, "Gethostbyname failed\n");
		exit(2);
	}

	*port = (unsigned short) atoi(arg2);

	/*
	 * Define o endereco IP e a porta do servidor
	 */
	server->sin_family      = AF_INET;
	server->sin_port        = htons(*port);
	server->sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
}

void socketConectar(int *s, struct sockaddr_in *server) {

	if ((*s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(3);
	}

	/* Estabelece conexao com o servidor */
	if (connect(*s, (struct sockaddr *)server, sizeof(*server)) < 0) {

		perror("Connect()");
		exit(4);
	}
}

void iniciaConexaoServer(int *s, unsigned short *port, struct sockaddr_in *server, struct sockaddr_in *client, int *namelen, char arg[]) {
	
	*port = (unsigned short) atoi(arg);
	
	if ((*s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(2);
	}

	server->sin_family = AF_INET;   
   	server->sin_port   = htons(*port);       
   	server->sin_addr.s_addr = INADDR_ANY;

		 
    	if (bind(*s, (struct sockaddr *)server, sizeof(*server)) < 0) {

       		perror("Bind()");
       		exit(3);
   	}

	if (listen(*s, 1) != 0) {

		perror("Listen()");
       		exit(4);
   	}

	*namelen = sizeof(*client);
}

void aceitaConexao(int *ns, int *s, struct sockaddr_in *client, int *namelen) {

	if ((*ns = accept(*s, (struct sockaddr *)client, (socklen_t *) namelen)) == -1) {
		perror("Accept()");
		exit(5);
	}
}

void enviarMensagem(int s, const void *enviar, size_t tamanho) {

	if (send(s, enviar, tamanho, 0) < 0) {

		perror("Send()");
		exit(5);
	}
}

void receberMensagem(int s, void *receber, size_t tamanho) {

	if (recv(s, receber, tamanho, 0) == -1) {

		perror("Recv()");
		exit(6);
	}
}