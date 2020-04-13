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

int iniciarMutex(pthread_mutex_t *mutex) {

	if (pthread_mutex_init(mutex, NULL) != 0)  {

		printf("falha iniciacao semaforo\n");
		exit(1);
	}
	else
		return 0;
}
