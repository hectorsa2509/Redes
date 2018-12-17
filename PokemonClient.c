#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <my_global.h>
#include <mysql.h>
#include "ErrorSalida.h"
#include "MessageDefinition.h"

#define SERV_PORT 9999            /* El servidor correra en este puerto */

#define RCVBUFSIZEC 8000
#define sqluser "root" //Usuario de MYSQL
#define sqlpass "NULL"  //Password MYSQL

void ErrorSalida(char *errorMessage);

int askYes_No();

void sendErrorCode(int clntSocket, char *message);

char *getPokemonName(unsigned char idPokemon);

void SQLError(MYSQL *con);

int checkGivenUserData(char *user,char *pass);

int getUserID(char *user);

int checkPokedex(int idUsuario, unsigned char idPokemon);

void registerInPokedex(int idUsuario, unsigned char idPokemon);


int main(int argc, char *argv[]) {
    if ((argc != 3)) { 
        fprintf(stderr, "Usage: %s <ServerIP> <Port>\n", argv[0]);
        exit(1);
    }
    /* Inicio de Sesion*/
    printf("Bienvenido al Centro Pokemon.\n");
    printf("Usuario previamente cargado en la base de datos...\n");
    printf("Inicia Sesion:\n");
    int failed = 1;
    char user[32];
    char pass[32];


    while (failed) {
        printf("Usuario:\n");
        scanf("%s", user);
        printf("Contraseña:\n");
        scanf("%s", pass);

        failed = checkGivenUserData(user,pass);
    }

    int idUsuario = getUserID(user);

    int sock;
    struct sockaddr_in pokeServAddr;
    unsigned short servPort;
    char *servIP;
    char *pokeString;
    char pokeBuffer[RCVBUFSIZEC];
    unsigned int pokeStringLen;
    int bytesRcvd, bytesSend;

    unsigned char code = 10;

    servIP = argv[1];
    servPort = atoi(argv[2]); /*cast String to int*/

    /*Creacion de la estructura del mensaje*/
    msg_type1 *msg_t1 = (struct msg_type1 *) (unsigned char *) malloc(sizeof(unsigned char));

    memcpy(msg_t1->code, &code, sizeof(msg_t1->code));

    /*Creamos el socket*/
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ErrorSalida("socket () failed");

    memset(&pokeServAddr, 0, sizeof(pokeServAddr));

    pokeServAddr.sin_family = AF_INET;
    pokeServAddr.sin_addr.s_addr = inet_addr(servIP);

    pokeServAddr.sin_port = htons(servPort);


    /* Asignamos los timeout para el envio y recibo respectivamente*/
    struct timeval timeout;
    timeout.tv_sec = 5;  /* wait 5 seconds */
    timeout.tv_usec = 0;  /* and 0 microseconds */

    /* Operaciones para recibir */

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
        ErrorSalida("Error\n");

    /* Operaciones para enviar */

    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
        ErrorSalida("Error\n");


    /*Conectamos el socket*/
    if ((connect(sock, (struct sockaddr *) &pokeServAddr, sizeof(pokeServAddr))) < 0)
        ErrorSalida("Conexion fallida");

    pokeStringLen = sizeof(msg_type1); //el PokeCliente solo envia mensajes de tipo 1

    /*Estado S0*/
    /*SENDING MSG*/

    if ((bytesSend = send(sock, msg_t1, sizeof(msg_type1), 0)) != pokeStringLen) //sending msg10
        ErrorSalida("send() se enviaron diferentes numero de bytes");
    else
        printf("Mensaje #10: Numero de bytes enviados al servidor: %i \n", bytesSend);

    /*RECEIVING MSG (STARTING POKEMON CAPTURE SECRET PROTOCOL: "Catch 'em all")*/
    //¿Capturar Pokemon X?
    /*Estado S2*/
    if ((bytesRcvd = recv(sock, pokeBuffer, RCVBUFSIZEC - 1, 0)) < 0)
        ErrorSalida("recv () error o la conexion termino");

    msg_type2 *msg20 = (struct msg_type2 *) pokeBuffer;
    code = *((unsigned char *) msg20->code);

    /* Expected code at this point: 20 */
    if (code != 20) {
        sendErrorCode(sock, "Error: Unexpected code, expected code 20");
        exit(-1);
    }

    unsigned char pokemon = *((unsigned char *) msg20->idPokemon);
    printf("¿Quieres atrapar el pokemon %s?\n", getPokemonName(pokemon));

    //checamos si el usario ya tiene el pokemon getPokemonName(pokemon)
    int already;
    if (already = checkPokedex(idUsuario, pokemon)) {
        printf("Ya tienes a %s registrado en tu Pokedex\n", getPokemonName(pokemon));
    }

    int captured;
    int canceled;
    int runOutOfTries;

    code = 30;
    msg_type1 *msg30 = (struct msg_type1 *) (unsigned char *) malloc(sizeof(unsigned char));
    memcpy(msg30->code, &code, sizeof(msg30->code));

    code = 31;
    msg_type1 *msg31 = (struct msg_type1 *) (unsigned char *) malloc(sizeof(unsigned char));
    memcpy(msg31->code, &code, sizeof(msg31->code));

    code = 32;
    msg_type1 *msg32 = (struct msg_type1 *) (unsigned char *) malloc(sizeof(unsigned char));
    memcpy(msg32->code, &code, sizeof(msg32->code));

    //obtener input standar del usuario (¿Quieres intentar capturar a X?)
    //if SI enviar mensaje 30
    if (askYes_No() == 0) { //SI
        printf("Atrapas al pokemon\n");
        if ((bytesSend = send(sock, msg30, sizeof(msg_type1), 0)) != pokeStringLen) //sending msg30
            ErrorSalida("send() se enviaron diferentes numero de bytes");
        else
            printf("Message 30: Bytes send to server: %i \n", bytesSend);

        captured = 0;
        canceled = 0;
        runOutOfTries = 0;
        //se intenta capturar, un n maximo numero de intentos
        while (!captured && !canceled && !runOutOfTries) {
            //Recibir mensaje del servidor (checar si es 21 o 22 o 23)
            if ((bytesRcvd = recv(sock, pokeBuffer, RCVBUFSIZEC - 1, 0)) < 0)
                ErrorSalida("recv () la conexion termino antes");

            msg_type1 *msgX = (struct msg_type1 *) pokeBuffer;
            code = *((unsigned char *) msgX->code);

            /*Estado S4 (se recibió 21)*/
            if (code == 21) {
                msg_type3 *msg21 = (struct msg_type3 *) pokeBuffer;
                printf("Te quedan %i intentos\n", *((int *) msg21->numAttemps));
                printf("¿Quieres intentarlo de nuevo?\n");
                if (askYes_No() == 0) {
                    printf("Un intento mas\n");
                    if ((bytesSend = send(sock, msg30, sizeof(msg_type1), 0)) != pokeStringLen) //sending msg30
                        ErrorSalida("send() se enviaron diferentes numero de bytes que los esperados");
                    else
                        printf("Message 30: Bytes send to server: %i \n", bytesSend);
                } else {
                    printf("Ya no se puede atrapar al pokemon\n");
                    if ((bytesSend = send(sock, msg31, sizeof(msg_type1), 0)) != pokeStringLen) //sending msg31
                        ErrorSalida("send() se enviaron diferentes numero de bytes");
                    else
                        printf("Message 31: Bytes send to server: %i \n", bytesSend);

                    canceled = 1;
                }
            } else if (code == 22) {/*Estado S5 (se recibió 22)*/
                msg_type4 *msg22 = (struct msg_type4 *) pokeBuffer;

                printf("Haz capturado a: %s\n", getPokemonName(
                        *((unsigned char *) msg22->idPokemon))); //NOTA: Es mejor que no se escriba solo el id, si no tambien el nombre del pokemon (base de datos?)
                int imageSizeR = *((int *) msg22->imageSize);
                char imageR[imageSizeR];
                char filename[32];
                snprintf(filename, sizeof(char) * 32, "%u.png", *((unsigned char *) msg22->idPokemon));
                FILE *image;
                image = fopen(filename, "w");
                fwrite(msg22->image, 1, sizeof(imageR), image);
                fclose(image);
                captured = 1;
            } else if (code == 23) { /*Estado S6 */
                printf("El pokemon ha escapado...\n");
                runOutOfTries = 1;
            } else {
                sendErrorCode(sock, "Error: Unexpected code, expected code 21, 22 or 23");
                exit(-1);
            }
        }
    } else {
        //cuando elige NO
        printf("Bueno espero volverte a ver :(\n");
        if ((bytesSend = send(sock, msg31, sizeof(msg_type1), 0)) != pokeStringLen)
            ErrorSalida("send() sent a different number of bytes than expected");
        else
            printf("Message 31: Bytes send to server: %i \n", bytesSend);
    }
    // se guarda el Pokemon con el usuario en la table Pokedex
    if (captured) {
        printf("Guardando en Pokedex y cerrando sesion.\n"); //Asegurarse que el pokemon no aparezca de nuevo o que la proxima vez que se encuentre se avise que ya fue capturado con anterioridad.
        if (!already)
            registerInPokedex(idUsuario, pokemon);
    } else if (canceled)
        printf("Cerrando Sesion.\n");
    else //runOutOfTries
        printf("Algun dia lo encontraremos de nuevo bye\n");

    if ((bytesSend = send(sock, msg32, sizeof(msg_type1), 0)) != pokeStringLen) //sending msg31
        ErrorSalida("send() se enviaron diferentes numero de bytes");
    else
        printf("Message 32: Bytes send to server: %i \n", bytesSend);

    /*Estado S7*/
    close(sock);

    exit(0);
}

//Returns 0 if yes, 1 if no
int askYes_No() {
    printf("Escoge:\n");
    int answer;
    int unanswered = 1;
    while (unanswered) {
        printf("0.Si\n");
        printf("1.No\n");

        scanf("%d", &answer);
        if (answer != 0 && answer != 1)
            printf("Opción no valida\n");
        else
            //Aqui esta asegurado que la respuesta es 1 o 0
            unanswered = 0;
    }

    return answer;
}

void sendErrorCode(int clntSocket, char *message) {
    int code = 40;

    msg_typeErr *err = (struct msg_typeErr *) (unsigned char *) malloc(sizeof(unsigned char));

    memcpy(err->code, &code, sizeof(err->code));
    memcpy(err->errString, &message, sizeof(err->errString));

    if (send(clntSocket, err, sizeof(err), 0) != sizeof(err))
        ErrorSalida("send() failed");
}

char *getPokemonName(unsigned char idPokemon) {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", sqluser, sqlpass, "PokemonBase", 0, NULL, 0) == NULL) {
        SQLError(con);
    }

    char query[128]; // The query buffer.
    snprintf(query, sizeof(char) * 128, "SELECT nombre FROM Pokemon WHERE idPokemon = %u;", idPokemon);

    if (mysql_query(con, query)) {
        SQLError(con);
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL) {
        SQLError(con);
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;
    row = mysql_fetch_row(result);

    char *returnvalue = row[0];

    mysql_free_result(result);
    mysql_close(con);

    if (returnvalue) {
        return returnvalue;
    } else {
        return "NULL";
    }
}

/*Este metodo checa la informacion del usuario
 recibiendo el nombre de usuario*/
int checkGivenUserData(char *user, char *pass) {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", sqluser, sqlpass, "PokemonBase", 0, NULL, 0) == NULL) {
        SQLError(con);
    }

    char query[128]; // The query buffer.
    snprintf(query, sizeof(char) * 128, "SELECT nombre_usuario FROM Usuario WHERE nombre_usuario = \"%s\";", user);
    snprintf(query, sizeof(char) * 128, "SELECT password FROM Usuario WHERE password = \"%s\";", pass);

    if (mysql_query(con, query)) {
        SQLError(con);
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL) {
        SQLError(con);
    }

    MYSQL_ROW row;
    int failed = 1;

    if ((row = mysql_fetch_row(result))) {
        failed = 0;
    }

    mysql_free_result(result);
    mysql_close(con);
    return failed;
}

//Regresa el id del usuario
int getUserID(char *user) {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", sqluser, sqlpass, "PokemonBase", 0, NULL, 0) == NULL) {
        SQLError(con);
    }

    char query[128]; // The query buffer.
    snprintf(query, sizeof(char) * 128, "SELECT idUsuario FROM Usuario WHERE nombre_usuario = \"%s\";", user);

    if (mysql_query(con, query)) {
        SQLError(con);
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL) {
        SQLError(con);
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;
    row = mysql_fetch_row(result);

    int returnvalue = atoi(row[0]);

    mysql_free_result(result);
    mysql_close(con);

    if (returnvalue) {
        return returnvalue;
    } else {
        return -1;
    }
}

// Se revisa si el usuario ya ha capturado el Pokemon que se ofrece capturar de nuevo. Se regresa 0 si no lo tiene y 1 si ya lo tiene
//Recibe dos parametros, el id del usuario y el id del pokemon, ambos valores numericos
int checkPokedex(int idUsuario, unsigned char idPokemon) {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", sqluser, sqlpass, "PokemonBase", 0, NULL, 0) == NULL) {
        SQLError(con);
    }

    char query[128]; // The query buffer.
    snprintf(query, sizeof(char) * 128, "SELECT idPokedex FROM Pokedex WHERE idPokemon = %u AND idUsuario = %d;",
             idPokemon, idUsuario);

    if (mysql_query(con, query)) {
        SQLError(con);
    }

    MYSQL_RES *result = mysql_store_result(con);

    if (result == NULL) {
        SQLError(con);
    }

    MYSQL_ROW row;
    int registered = 0;

    if ((row = mysql_fetch_row(result))) {
        registered = 1;
    }

    mysql_free_result(result);
    mysql_close(con);
    return registered;
}

// Al ser capturado el Pokemon se registra en la tabla Pokedex, asociado con el usuario que lo capturo
//Para registrarlo en Pokedex el metodo recibe dos parametros: id del Pokemon y id del usuario.
void registerInPokedex(int idUsuario, unsigned char idPokemon) {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "127.0.0.1", sqluser, sqlpass, "PokemonBase", 0, NULL, 0) == NULL) {
        SQLError(con);
    }

    char query[128]; // The query buffer.
    snprintf(query, sizeof(char) * 128, "INSERT INTO Pokedex (idUsuario,idPokemon) VALUES (%d,%u);", idUsuario,
             idPokemon);

    if (mysql_query(con, query)) {
        SQLError(con);
    }

    mysql_close(con);
    return;
}

//Error en base de datos
void SQLError(MYSQL *con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}
