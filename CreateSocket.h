#include <sys/socket.h>   /* Para el uso de los metodos  socket(), bind() and connect() */
#include <arpa/inet.h>
#include <string.h>

#define MAXPENDING 5      /* Maximo de peticiones de conexion */

void ErrorSalida(char *errorMessage);   /* Funcion de error de la aplicacion */

int CreateSocket(unsigned short port) {
    int sock;                        /* Creamos el socket */
    struct sockaddr_in servAddr; /* direccion local */

    /* Se crea un socket para las conecciones entrantes */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorSalida("socket() failed");


    /* Constructor para una direccion local */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;            /* Direccion de internet familiar */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Cualquier interface entrante */
    servAddr.sin_port = htons(port);        /* Puerto dado */

    /* Bind a la direccion local */
    if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        ErrorSalida("bind() failed");

    /* Marcamos el socket para escuchar las conecciones entrantes */
    if (listen(sock, MAXPENDING) < 0)
        ErrorSalida("listen() failed");

    return sock;

}
