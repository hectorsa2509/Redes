#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void ErrorSalida(char *errorMEssage); /*  Manejo de errores */


int AcceptTCPConnection(int servSock) {

    int clntSock;                /* Socket Cliente */
    struct sockaddr_in clntAddr; /* Direccion Cliente */
    unsigned int clntLen;        /* Longitud de la estructura de datos de la dirección del cliente*/


    /*Se estable el tamano del parametro de entrada-salida*/
    clntLen = sizeof(clntAddr);

    /* Esperando conexion del cliente */
    if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntLen)) < 0)
        ErrorSalida("accept() failed");

    /* Conexion establecida */
    printf("Servicio Cliente %s\n", inet_ntoa(clntAddr.sin_addr));

    return clntSock;
}
