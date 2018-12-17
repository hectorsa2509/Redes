#include <stdio.h>
#include <stdlib.h>

/*
Metodo que se usara para terminar la aplicacion
Saldra un mensaje diciendonos cual fue el error (errorMessage) 
*/
void ErrorSalida(char *errorMessage) {
    /* perror() produce un mensaje hacia la stderr
       que es la salida estándar de error */
    perror(errorMessage);
    exit(1);
}
