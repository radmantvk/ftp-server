#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include "usage.h"
#include <pthread.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "handleCommand.h"


char send_buffer[1000], receive_buffer[1000];
int n, login;
char user[] = "USER";
char cs317[] = "CS317";

char* getIPAddr(char *IPbuffer) {
  char hostbuffer[256];
  
  struct hostent *host_entry;
  int hostname;

  hostname = gethostname(hostbuffer, sizeof(hostbuffer));
  
  host_entry = gethostbyname(hostbuffer);
  
  IPbuffer = inet_ntoa(*((struct in_addr*)
                        host_entry->h_addr_list[0]));

  return IPbuffer;
}

void sendMessage(void * client_sock, char* message) {
  int len = strlen(message);
  char str[len + 1];
  sprintf(str, "%s", message);
  send(*(int*)client_sock, strcat(str, "\n"), len+1, 0);
}

char* toUp(char* st) {
  char* n = st;
  while(*n) {
    *n = toupper((unsigned char) *n);
    n++;
  }
  return st;
}

char* tokenize(char* receive_buffer) {
  char *p = strtok(receive_buffer, " ");
  while(p != NULL) {
    p = strtok(NULL, " ");
  }
  return p;
}

// Handles each connection 

void *handleConnection(void* client_sock)
{
  // This establishes the root directory at the beginning of connection
  // When the connection closes, it resets the server to the root dir at the end
  int bytes;
  char *first_line, *second_line;
  sendMessage(client_sock, "220 Successful connection.");
  char currDir[200] = "";
  getcwd(currDir, 200);
  int z = 199;
  while (currDir[z] != '\0'){
    z--;
  }
  char root[z+1];
  memmove(root, currDir, z);
  root[z+1] = '\0';
  
  while (1) {
    n = 0;
    while (1) {
      bytes = recv(*(int*)client_sock, &receive_buffer[n], 1, 0);
      //PROCESS REQUEST
      if ( bytes <= 0 ) {
        break;
      } 
      if (receive_buffer[n] == '\n') { 
        receive_buffer[n] = '\0';
        break;
      }
      n++; 
      }
      struct ftp_cmd cmd = parseCmd(receive_buffer);
    
      int shouldQuit = handleCommand(client_sock, cmd, root);
      if (shouldQuit == 1) {
        return NULL;
      }
    }
  chdir(root);
  free(client_sock);
  return NULL;
}


