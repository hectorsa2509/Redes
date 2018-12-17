#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ErrorSalida.h"
#include "AcceptTCPConnection.h"
#include "CreateSocket.h"
#include "HandleTCPClient.h"

void ErrorSalida(char *errorMessage);

void HandleTCPClient(int clntSocket);

int CreateSocket(unsigned short port);            /* Creamos l socket TCP*/
int AcceptTCPConnection(int servSock);          /* Acepta la conexion TCP*/