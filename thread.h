#ifndef thread
#define thread

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

void iniciarMutex(pthread_mutex_t *mutex);
int criarThread(pthread_t tratarClientes, void *tratamento, void *informacoes);

#endif
