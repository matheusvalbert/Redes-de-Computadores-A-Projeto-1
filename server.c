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
#include <netdb.h>

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

	port = (unsigned short) atoi(arg);

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(2);
	}

	server.sin_family = AF_INET;   
    server.sin_port   = htons(port);       
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {

       	perror("Bind()");
       	exit(3);
    }

    if (listen(s, 1) != 0) {

		perror("Listen()");
       	exit(4);
   	}

   	namelen = sizeof(client);

	while(1) {

	    if ((ns = accept(s, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1) {
	        perror("Accept()");
	        exit(5);
	    }

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
	int comando;
	printf("conexao aceita - IP: %s - Porta: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

	while(1) {

		if (recv(ns, &comando, sizeof(int), 0) == -1) {

			perror("Recv()");
			exit(6);
		}

		if(comando == 1)
			listar(ns, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		else if(comando == 2)
			receber(ns, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		else if(comando == 3)
			enviar(ns, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		else if(comando == 4) {

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
	filesize = strlen(file_names);
	file_names[filesize] = '\n';
	file_names[filesize + 1] = '\0';
	filesize++;
	if (send(ns, &filesize, sizeof(int), 0) < 0) {

		perror("Send()");
		exit(5);
	}
	if (send(ns, file_names, strlen(file_names), 0) < 0) {

		perror("Send()");
		exit(5);
	}
	closedir(dir);
}

void receber(int ns, char ip[], int p) {

	unsigned short port;
	struct hostent *hostnm;
    struct sockaddr_in server;
	int sData;
	char argument[50];
	char host[50];
	int len;

	int porta;
	if (recv(ns, &porta, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	sprintf(argument, "%d", porta);

	if (recv(ns, &len, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	
	if (recv(ns, host, len, 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	hostnm = gethostbyname(host);
	if (hostnm == (struct hostent *) 0)  {

		fprintf(stderr, "Gethostbyname failed\n");
		exit(2);
	}

	port = (unsigned short) porta;

	server.sin_family      = AF_INET;
	server.sin_port        = htons(port);
	server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);


	if ((sData = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(3);
	}

	if (connect(sData, (struct sockaddr *)&server, sizeof(server)) < 0) {

		perror("Connect()");
		exit(4);
	}


	char arg[50];
	printf("receber - IP: %s - Porta: %d\n", ip, p);
	int argTam;
	if (recv(ns, &argTam, sizeof(argTam), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	if (recv(ns, arg, argTam, 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	arg[argTam] = '\0';

	int size;
	unsigned char buffer[1024];

	FILE *ptr;
	ptr = fopen(arg,"rb");

	if(ptr == NULL){

		perror("Open()");
		exit(0);
	}

	fseek(ptr, 0, SEEK_END);
	size = (int)ftell(ptr);
	rewind(ptr);

	if (send(ns, &size, sizeof(size), 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	int nvezes = size/1024;

	while(nvezes != 0) {

		fread(buffer,1024,1,ptr);
		if (send(sData, buffer, 1024*sizeof(char), 0) == -1) {

			perror("Recv()");
			exit(6);
		}
		nvezes--;
	}

	if(size%1024 != 0) {

		fread(buffer,size%1024,1,ptr);
		if (send(sData, buffer, (size%1024)*sizeof(char), 0) == -1) {

			perror("Recv()");
			exit(6);
		}
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
	char host[50];
	int len;

	int porta;
	if (recv(ns, &porta, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	sprintf(argument, "%d", porta);

	if (recv(ns, &len, sizeof(int), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	
	if (recv(ns, host, len, 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	hostnm = gethostbyname(host);
	if (hostnm == (struct hostent *) 0)  {

		fprintf(stderr, "Gethostbyname failed\n");
		exit(2);
	}

	port = (unsigned short) porta;

	server.sin_family      = AF_INET;
	server.sin_port        = htons(port);
	server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

	if ((sData = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

		perror("Socket()");
		exit(3);
	}
	
	if (connect(sData, (struct sockaddr *)&server, sizeof(server)) < 0) {

		perror("Connect()");
		exit(4);
	}

	char arg[50];
	unsigned char buffer[1024];
	int size, tamanho;
	printf("enviar - IP: %s - Porta: %d\n", ip, p);
	int argTam;
	if (recv(ns, &argTam, sizeof(argTam), 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	if (recv(ns, arg, argTam, 0) == -1) {

		perror("Recv()");
		exit(6);
	}
	if (recv(ns, &size, sizeof(size), 0) == -1) {

		perror("Recv()");
		exit(6);
	}

	FILE *ptr;

	ptr = fopen(arg,"wb");

	int nvezes = size/1024;

	while(nvezes != 0) {

		if (recv(sData, buffer, 1024*sizeof(char), 0) == -1) {

			perror("Recv()");
			exit(6);
		}
		fwrite(buffer,1024,1,ptr);
		nvezes--;
	}

	if(size%1024 != 0) {

		if (recv(sData, buffer, (size%1024)*sizeof(char), 0) == -1) {

			perror("Recv()");
			exit(6);
		}
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
