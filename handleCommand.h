
#ifndef __HANDLECOMMANDH__
#define __HANDLECOMMANDH__

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
#include "handleConnection.h"
#include <time.h>


 struct ftp_cmd {
    char* cmd;
    char* token;
 } ftp_cmd;

// void *handleCommand(void* client_sock);

int handleCommand(void* client_sock, struct ftp_cmd cmd, char* root);

int cmdExists(struct ftp_cmd cmd);

struct ftp_cmd parseCmd(char* buf);

#endif