#include "TCPServer.h" 
#include <pthread.h>         


#define SERV_PORT 9999            /* El servidor correra en el puerto 9999 */

void *ThreadMain(void *arg); 

/* Thread Cliente */
struct ThreadArgs {
    int clntSock;
};

int main(int argc, char *argv[]) {
    int servSock;                   /* Socket para el servidor */
    int clntSock;                   /* Socket para el cliente */
    unsigned short servPort;        /* Puerto */
    pthread_t threadID;  
    struct ThreadArgs *threadArgs;  

    /* Creamos el puerto del servidor */
    servPort = SERV_PORT;
    servSock = CreateSocket(servPort);

    for (;;) { /* Corre por siempre xd */
        clntSock = AcceptTCPConnection(servSock);

        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL)
            ErrorSalida("malloc() failed");
        threadArgs->clntSock = clntSock;

        /* Creamos el thread para el cliente */
        if (pthread_create(&threadID, NULL, ThreadMain, (void *) threadArgs) != 0)
            ErrorSalida("pthread_create() failed");

    }
}

void *ThreadMain(void *threadArgs) {
    int clntSock;

    /* Garantizas que los recursos de los thread se desasignen a la devolución */
    pthread_detach(pthread_self());
    clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);  

    HandleTCPClient(clntSock);

    return (NULL);
}

