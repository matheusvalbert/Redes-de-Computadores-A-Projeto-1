#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

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

	hostnm = gethostbyname(arg1);
	if (hostnm == (struct hostent *) 0)  {

		fprintf(stderr, "Gethostbyname failed\n");
		exit(2);
	}

	*port = (unsigned short) atoi(arg2);

	server->sin_family      = AF_INET;
	server->sin_port        = htons(*port);
	server->sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

	if ((*s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(3);
	}

	if (connect(*s, (struct sockaddr *)server, sizeof(*server)) < 0) {

		perror("Connect()");
		exit(4);
	}

	strcpy(host, arg1);
	printf("%s\n", host);
	printf("conexao aceita\n");
	return true;
}

void listar(int s) {
	char file_names[200];
	int num;

	int op = 1;
	if (send(s, &op, sizeof(int), 0) < 0) {

		perror("Send()");
		exit(5);
	}
	if (recv(s, &num, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	if (recv(s, file_names, num, 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	for(int i=0;i<num;i++)
	{
		if(file_names[i] == ' ')
			printf("\n");
		else
			printf("%c", file_names[i]);
	}
}

void receber(int s, char arg1[], char arg2[]) {

	int op = 2;

	if (send(s, &op, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	unsigned short port;
	int nsData, namelen, sData;
	struct sockaddr_in client;
	struct sockaddr_in server;
	char argument[50];

	int porta = 5000 + cout;
	sprintf(argument, "%d", porta);

	if (send(s, &porta, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	cout++;

	int len = strlen(host);
	if (send(s, &len, sizeof(int), 0) < 0) {

		perror("Send()");
		exit(5);
	}   	

   	if (send(s, host, strlen(host), 0) < 0) {

		perror("Send()");
		exit(5);
	}

	port = (unsigned short) porta;

	if ((sData = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(2);
	}

	server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sData, (struct sockaddr *)&server, sizeof(server)) < 0) {

       	perror("Bind()");
       	exit(3);
    }

    if (listen(sData, 1) != 0) {

		perror("Listen()");
       	exit(4);
   	}

   	namelen = sizeof(client);


    if ((nsData = accept(sData, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1) {
        perror("Accept()");
        exit(5);
    }

	int argTam = strlen(arg1);
	if (send(s, &argTam, sizeof(argTam), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	if (send(s, arg1, strlen(arg1), 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	unsigned char buffer[1024];
	int size, tamanho;

	if (recv(s, &size, sizeof(size), 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	FILE *ptr;

	ptr = fopen(arg2,"wb");

	int nvezes = size/1024;

	while(nvezes != 0) {

		if (recv(nsData, buffer, 1024*sizeof(char), 0) == -1) {

			perror("Recv()");
			exit(6);
		}
		fwrite(buffer,1024,1,ptr);
		nvezes--;
	}

	if(size%1024 != 0) {

		if (recv(nsData, buffer, (size%1024)*sizeof(char), 0) == -1) {

			perror("Recv()");
			exit(6);
		}
		fwrite(buffer,size%1024,1,ptr);
	}

	fclose(ptr);
	close(nsData);
	close(sData);

	printf("arquivo recebido - Nome no servidor: %s - Nome no cliente: %s\n", arg1, arg2);
}

void enviar(int s, char arg1[], char arg2[]) {

	int op = 3;


	arg1[strlen(arg1)] = '\0';
	arg2[strlen(arg2)] = '\0';

	if (send(s, &op, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	unsigned short port;
	int nsData, namelen, sData;
	struct sockaddr_in client;
	struct sockaddr_in server;
	char argument[50];
	int len;

	int porta = 5000 + cout;
	sprintf(argument, "%d", porta);
	if (send(s, &porta, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	cout++;

	len = strlen(host);

	if (send(s, &len, sizeof(int), 0) < 0) {

		perror("Send()");
		exit(5);
	}	

	if (send(s, host, strlen(host), 0) < 0) {

		perror("Send()");
		exit(5);
	}

	int argTam = strlen(arg2);
	if (send(s, &argTam, sizeof(int), 0) < 0) {

		perror("Send()");
		exit(5);
	}
	if (send(s, arg2, strlen(arg2), 0) < 0) {

		perror("Send()");
		exit(5);
	}

	port = (unsigned short) porta;

	if ((sData = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(2);
	}

	server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sData, (struct sockaddr *)&server, sizeof(server)) < 0) {

       	perror("Bind()");
       	exit(3);
    }

    if (listen(sData, 1) != 0) {

		perror("Listen()");
       	exit(4);
   	}

   	namelen = sizeof(client);


    if ((nsData = accept(sData, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1) {
        
        perror("Accept()");
        exit(5);
    }

	int size;
	unsigned char buffer[1024];

	FILE *ptr;
	ptr = fopen(arg1,"rb");
	if(ptr == NULL){

		perror("Open()");
		exit(0);
	}

	fseek(ptr, 0, SEEK_END);
	size = (int)ftell(ptr);
	rewind(ptr);

	if (send(s, &size, sizeof(size), 0) < 0) {

		perror("Send()");
		exit(5);
	}

	int nvezes = size/1024;

	while(nvezes != 0) {

		fread(buffer,1024,1,ptr);
		if (send(nsData, buffer, 1024*sizeof(char), 0) < 0) {

			perror("Send()");
			exit(5);
		}
		nvezes--;
	}

	if(size%1024 != 0) {

		fread(buffer,size%1024,1,ptr);
		if (send(nsData, buffer, (size%1024)*sizeof(char), 0) < 0) {

			perror("Send()");
			exit(5);
		}
	}

	fclose(ptr);
	close(nsData);
	close(sData);

	printf("arquivo enviado - Nome no cliente: %s - Nome no servidor: %s\n", arg1, arg2);
}

void encerrar(int s) {

	int op = 4;
	if (send(s, &op, sizeof(4), 0) < 0) {

		perror("Send()");
		exit(5);
	}
	close(s);
}
