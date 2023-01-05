#ifndef _HANDLECONNECTIONH__
#define _HANDLECONNECTIONH__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "dir.h"
#include "usage.h"
#include <pthread.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "handleCommand.h"

// void send_code(void * client_sock, int code);
void sendMessage(void * client_sock, char* message);
char* toUp(char* st);

char* getIPAddr(char *IPbuffer);

char* tokenize(char* receive_buffer);

void *handleConnection(void* client_sock);



#endif