/*Estructura para los mensajes 10,30,31,32 y 23*/
typedef struct msg_type1 {
    unsigned char code[1];
} msg_type1;

/*mensajes 20 */

typedef struct msg_type2 {
    unsigned char code[1];
    unsigned char idPokemon[1];
} msg_type2;

/*mensajes 21 */

typedef struct msg_type3 {
    unsigned char code[1];
    unsigned char idPokemon[1];
    unsigned char numAttemps[1];
} msg_type3;

/*mensajes 22 */

typedef struct msg_type4 {
    unsigned char code[1];
    unsigned char idPokemon[1];
    unsigned char imageSize[4];
    unsigned char image[8000];
} msg_type4;

/*mensajes 40 (Error) */

typedef struct msg_typeErr {
    unsigned char code[1];
    unsigned char errString[32];
} msg_typeErr;



