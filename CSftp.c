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
#include "handleCommand.h"

// Here is an example of how to use the above function. It also shows
// one how to get the arguments passed on the command line.
/* this function is run by the second thread */

// Main function, handles sockets and pthreads 
// 
// Structure of code 
// CSftp -> handleConnection -> handleCommand
//
int main(int argc, char **argv) {
  int server_sock, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[1024];
  char *IPbuffer;

  // Check the command line arguments
  if (argc != 2) {
    usage(argv[0]);
    return -1;
  }

  IPbuffer = getIPAddr(IPbuffer);
  printf("%s", IPbuffer);
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  
  // Check to determine if socket was created successfully
  if (server_sock == -1) {
    printf("socket creation failed!\n\n");
    return -1;
  }
  
  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IPbuffer); //inet_addr(IPbuffer);
	server_addr.sin_port = htons((uint16_t) atoi(argv[1])); 
 
  if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    printf("Could not bind\n");
    return -1;
  }

  listen(server_sock , 5);
  pthread_t child;
  while (1) {
    addr_size = sizeof(client_addr);
    printf("\n2\n");
    fflush(stdout);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, (socklen_t*)&addr_size);
    
    printf("HELLO");
    fflush(stdout);
    pthread_create(&child, NULL, handleConnection, &client_sock);
    pthread_join(child, NULL);
  }
  close(server_sock);
  return 0;
}
