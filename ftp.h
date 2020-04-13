#ifndef ftp
#define ftp

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void iniciaConexaoClient(char arg1[], char arg2[], unsigned short *port, struct hostent *hostnm, struct sockaddr_in *server);
void socketConectar(int *s, struct sockaddr_in *server);
void iniciaConexaoServer(int *s, unsigned short *port, struct sockaddr_in *server, struct sockaddr_in *client, int *namelen, char arg[]);
void aceitaConexao(int *ns, int *s, struct sockaddr_in *client, int *namelen);
void enviarMensagem(int s, char enviar[], int tamanho);
void receberMensagem(int s, char receber[], int tamanho);

#endif
