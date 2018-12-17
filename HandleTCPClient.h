#include <stdio.h>             /* for printf() and fprintf() */
#include <sys/socket.h>        /* for recv() and send() */
#include <unistd.h>            /* for close() */
#include <stdlib.h>            /* for rand() and srand() */
#include <time.h>              /* for time() */
#include "MessageDefinition.h" /* structures for messages */

#define RCVBUFSIZE 1          /* Siempre recibe solo un byte */

void ErrorSalida(char *errorMessage);            /* Error handling function */
void interaction(unsigned char *code, int *state, int clntSocket, int *numAttemps,
                 int randomPoke);  /* Code/state handling function */

void HandleTCPClient(int clntSocket) {

    unsigned char code = 0;         /* Codigo para el mensaje */
    int state = 0;                  /* Estado actual del servidor */
    int numAttemps = 3;              /* numero de intentos restantes */
    srand((unsigned) time(NULL));    /* obtencion aleatoria */
    int randomPoke = rand() % 9;    /* numero aleatorio de 0 a 8, el cual el cliente intenta capturar */

    /* Iteracion entre los estados, envia y recibe datos hara el final de la conexion(estado 7)*/
    while (state != 7) {

        interaction(&code, &state, clntSocket, &numAttemps, randomPoke);

    }

    close(clntSocket);    /* Close client socket */

    printf("Termina la conexión, el socket cerró\n");

};


/*
 * Maneja las iteraciones basadas en el codigo de recibido y en el estado del thread, es decir, la funcion de trancision
 * de la maquina de estados, tambien envia una respuesta.
 */
void interaction(unsigned char *code, int *state, int clntSocket, int *numAttemps, int randomPoke) {

    char serverResponse[22];     /* mensaje de confirmacion*/
    char buffer[RCVBUFSIZE];     /* buffer para el mensaje recibido */
    int recvMsgSize;             /* tamaño del mensaje recibido */
    int codeToSend;              /* codigo del mensaje a enviar*/

    /* estructuras para almacenar mensajes para enviar */

    msg_type1 *msgtype1;
    msg_type2 *msgtype2;
    msg_type3 *msgtype3;
    msg_type4 *msgtype4;

    msg_type2 *getRandomPoke(int randomPoke);
    msg_type3 *getType3Message(int randomPoke, int *attemps);
    msg_type4 *getType4Message(int randomPoke);
    void sendErrorCode(int clntSocket, char *message);        /* envia un codigo de error 40 al cliente */


    /* Cambia el estado 0 al estado 1 */

    if (*code == 0 && *state == 0) {

        /* Mensaje recibido */

        if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
            ErrorSalida("recv() failed");

        msg_type1 *msg = (struct msg_type1 *) buffer;
        *code = *((unsigned char *) msg->code);
        printf("Un cliente desea capturar un Pokemon\n");

        /* Codigo esperado: 10 */
        if (*code != 10) {
            sendErrorCode(clntSocket, "Codigo esperado: 10");
            *state = 7;
            return;
        }

        *state = 1;

        /* Poke aleatorio */
        msgtype2 = getRandomPoke(randomPoke);

        if (send(clntSocket, msgtype2, sizeof(struct msg_type2), 0) != sizeof(struct msg_type2))
            ErrorSalida("send()");

        printf("Capturar a %i\n", randomPoke);

    } else if (*code == 10 && *state == 1 || /* Mismo comportamiento */
               *code == 30 && *state == 1 ||
               *code == 31 && *state == 1) {

        /* mensaje recibido */

        if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
            ErrorSalida("recv() failed");

        msg_type1 *msg = (struct msg_type1 *) buffer;
        *code = *((unsigned char *) msg->code);

        /* Codigo esperado: 30 o 31 */
        if (*code != 30 && *code != 31) {
            sendErrorCode(clntSocket, "Codigo esperado: 30 o 31");
            *state = 7;
            return;
        }


        /* Con el codigo 31 solo envia el codigo 32 y termina la sesion */

        if (*code == 31) {

            /* construye el mensaje */
            codeToSend = 32;
            msgtype1 = (struct msg_type1 *) (unsigned char *) malloc(sizeof(unsigned char));
            memcpy(msgtype1->code, &codeToSend, sizeof(msgtype1->code));

            /* Lo envia */

            if (send(clntSocket, msgtype1, sizeof(struct msg_type1), 0) != sizeof(struct msg_type1))
                ErrorSalida("send() failed");

            printf("Adios :(\n");

            /* Cambia el estado al estado final. */
            *state = 7;

        } else { /* Codigo 30, el cliente quiere atrapar el poke */

            /* ¿Se consigue el poke? */
            int r = random() % 100;
            if (r >= 55) {

                /* Lo consigue/atrapa */
                msgtype4 = getType4Message(randomPoke);

                /* Envia la respuesta */
                if (send(clntSocket, msgtype4, sizeof(struct msg_type4), 0) != sizeof(struct msg_type4))
                    ErrorSalida("send() failed");

                printf("Enviando imagen...\n");

                /* Cambio de estado al 6 */
                *state = 6;

            } else {

                /* Revisa si el ultimo intento fallo */
                if (*numAttemps == 1) {
                    /* No lo consiguio. */

                    /* Construye el mensaje. */
                    codeToSend = 23;
                    msgtype1 = (struct msg_type1 *) (unsigned char *) malloc(sizeof(unsigned char));
                    memcpy(msgtype1->code, &codeToSend, sizeof(msgtype1->code));

                    /* Lo envia */

                    printf("No lo logró, enviando respuesta...\n");

                    if (send(clntSocket, msgtype1, sizeof(struct msg_type1), 0) != sizeof(struct msg_type1))
                        ErrorSalida("send() failed");

                    /* Cambio de estado. */
                    *state = 6;

                } else {
                    /* Aun hay intentos, solo decrementa y envia mensaje. */
                    printf("Quedan %i intentos\n", *numAttemps - 1);
                    *numAttemps = *numAttemps - 1;
                    msgtype3 = getType3Message(randomPoke, numAttemps);
                    /* Envia la respuesta */
                    if (send(clntSocket, msgtype3, sizeof(struct msg_type3), 0) != sizeof(struct msg_type3))
                        ErrorSalida("send() failed");

                    /* Mismo estado */
                }
            }
        }

    } else if (*state == 6) { /* Espera un mensaje 32 del cliente. */

        /* mensaje recibido */

        if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
            ErrorSalida("recv() failed");

        msg_type1 *msg = (struct msg_type1 *) buffer;
        *code = *((unsigned char *) msg->code);

        /* Codigo esperado: 32 */
        if (*code != 32) {
            sendErrorCode(clntSocket, "Codigo esperado: 32");
            printf("Error:No se recibió un mensaje de termino de sesión\n");
            *state = 7;
            return;
        } else
            *state = 7;

    } else if (*code == 32) { /* Termina sesion */

        *state = 7;

    }

}

/* Mensaje con un codigo de un pokemon aleatorio.*/

msg_type2 *getRandomPoke(int randomPoke) {

    int code = 20;
    msg_type2 *poke = (struct msg_type2 *) (unsigned char *) malloc(sizeof(unsigned char));
    memcpy(poke->code, &code, sizeof(poke->code));
    memcpy(poke->idPokemon, &randomPoke, sizeof(poke->idPokemon));
    return poke;
}

/* Mensaje con un codigo de un pokemon aleatorio y numero de intentos. */

msg_type3 *getType3Message(int randomPoke, int *attemps) {

    int code = 21;
    msg_type3 *msg = (struct msg_type3 *) (unsigned char *) malloc(sizeof(unsigned char));
    memcpy(msg->code, &code, sizeof(msg->code));
    memcpy(msg->idPokemon, &randomPoke, sizeof(msg->idPokemon));
    memcpy(msg->numAttemps, attemps, sizeof(msg->numAttemps));
    return msg;

}

/* Mensaje con un codigo de un pokemon aleatorio y numero de intentos y la imagen. */

msg_type4 *getType4Message(int randomPoke) {

    char *getPokeFile(int rPoke);

    int code = 22;
    int imageSize;
    unsigned char *image;
    FILE *picture;
    char *pokeFile = getPokeFile(randomPoke);

    //Tamaño de la imagen
    if ((picture = fopen(pokeFile, "r")) == 0)
        ErrorSalida("fopen() failed");
    fseek(picture, 0, SEEK_END);
    imageSize = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    //Almacena la imagen como un arreglo de bytes.
    image = (unsigned char *) malloc(imageSize);
    if (!image) {
        ErrorSalida("malloc() failed");
    }
    fread(image, imageSize, sizeof(unsigned char), picture);

    /* Construye y regresa el mensaje */

    msg_type4 *msg = (struct msg_type4 *) (unsigned char *) malloc(sizeof(unsigned char));
    memcpy(msg->code, &code, sizeof(msg->code));
    memcpy(msg->idPokemon, &randomPoke, sizeof(msg->idPokemon));
    memcpy(msg->imageSize, &imageSize, sizeof(msg->imageSize));
    memcpy(msg->image, image, sizeof(msg->image));

    return msg;

}

/* implements ErrorSalida when function send() from sys/socket.h failed*/
void sendErrorCode(int clntSocket, char *message) {
    int code = 40;

    msg_typeErr *err = (struct msg_typeErr *) (unsigned char *) malloc(sizeof(unsigned char));

    memcpy(err->code, &code, sizeof(err->code));
    memcpy(err->errString, &message, sizeof(err->errString));

    if (send(clntSocket, err, sizeof(err), 0) != sizeof(err))
        ErrorSalida("send() failed");
}

/* Nombre del archivo que contiene la imagen del pokemon .*/

char *getPokeFile(int rPoke) {

    switch (rPoke) {

        case 0 :

            return "images/bulbasaur.png";
            break;

        case 1 :

            return "images/ivysaur.png";
            break;

        case 2 :

            return "images/venusaur.png";
            break;

        case 3 :

            return "images/squirtle.png";
            break;

        case 4 :

            return "images/wartortle.png";
            break;

        case 5 :

            return "images/blastoise.png";
            break;

        case 6 :

            return "images/charmander.png";
            break;

        case 7 :

            return "images/charmeleon.png";
            break;

        default :

            return "images/charizard.png";

    }

}
