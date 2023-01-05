
#include "handleCommand.h"
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>

// SECTION 1: Helpful global variables

int loggedIn = 0;
int bytes;
char send_buf[1000], receive_buf[1000];
char deli = ' ';
int pasvMode = 0;
char* IP;
int dataSock;
struct sockaddr_in client_data_addr;
char type;

// SECTION 2: Helper functions for commands

void handleUser(char * nameInput,void* client_sock) {
    if (nameInput == NULL) {
        sendMessage(client_sock, "503 Bad sequence of commands.");
        return;
    }
    if (strcmp(nameInput, "cs317") == 0) {
        loggedIn = 1;
        sendMessage(client_sock, "230 User logged in.");
    }
}
void handleQuit(void* client_sock) {
        sendMessage(client_sock, "221 Service closing control connection.");
        if (dataSock) {
            close(dataSock);
            dataSock = -1;
        }
        close(*(int*)client_sock);
}

void handleCWD(char * token, void* client_sock) {
    if (token == NULL) {
        sendMessage(client_sock, "501 Syntax error in parameters or arguments");
    }
    if((strstr(token, "./") != NULL || chdir(token) == -1)) {
        sendMessage(client_sock, "550 Requested action not taken.");
    } else {
        sendMessage(client_sock, "250 CWD completed");
    }
}

void handleCDUP(void* client_sock, char* root) {
    char currDir[200] = "";
    getcwd(currDir, 200);
    int z = 199;
    while (currDir[z] != '/'){
        z--;
    }
    if(strlen(root) > z) {
        sendMessage(client_sock, "550 Requested action not taken.");
        return;
    }
    char newPath[z+1];
    memmove(newPath, currDir, z);
    newPath[z] = '\0';

    if((strstr(newPath, "./") != NULL || strstr(newPath, "..") != NULL || chdir(newPath) == -1)) {
        sendMessage(client_sock, "550 Requested action not taken.");
    } else {
        sendMessage(client_sock, "250 Directory has been changed.");
    }
}

void handleSTRU(char* arg, void* client_sock) {
    if (arg == NULL) {
        sendMessage(client_sock, "501 Syntax error in parameters or arguments");
    } else if (strcmp(arg, "File") == 0 || strcmp(arg, "F") == 0) {
        sendMessage(client_sock, "200 Command okay");
    } else {
        sendMessage(client_sock, "504 Command not implemented for that parameter");
    }
}

void handleMODE(char* arg, void* client_sock) {
    if (arg == NULL) {
        sendMessage(client_sock, "503 Bad sequence of commands");
    } else if (strcmp(arg, "Stream") == 0 || strcmp(arg, "S") == 0) {
        sendMessage(client_sock, "200 MODE is now Stream.");
    } else {
        sendMessage(client_sock, "504 Command not implemented for that parameter.");
    }
}

void handlePASV(void* client_sock) {
    IP = getIPAddr(IP);
    
    dataSock = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSock == -1) {
        printf("socket creation failed!\n");
    }
    fcntl(dataSock, F_SETFL, O_NONBLOCK);
    struct sockaddr_in server_addr, client_addr;
    
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP); //inet_addr(IPbuffer);
    uint16_t port_num = 8888;
	server_addr.sin_port = htons((uint16_t) port_num); 
    while (bind(dataSock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        port_num++;
	    server_addr.sin_port = htons((uint16_t) port_num); 
    }
    char * h1 = strtok(IP, ".");
    char * h2 = strtok(NULL, ".");
    char * h3 = strtok(NULL, ".");
    char * h4 = strtok(NULL, ".");
    short p1s =  port_num / 256;
    short p2s = port_num % 256; 
    char temp[80];
    char temp1[100] = "227 Entering Passive Mode ";

    sprintf(temp, "(%s,%s,%s,%s,%hu,%hu)", h1, h2, h3, h4, p1s, p2s);
    strcat(temp1, temp);
    sendMessage(client_sock, temp1); // 207 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
    listen(dataSock , 1);
    socklen_t addr_size;
    int trigger = 300000;
    clock_t before = clock();
    int d_sock = -1;
    while (((clock() - before)/100) < trigger && d_sock <= 0) {
        d_sock = accept(dataSock, (struct sockaddr*)&client_data_addr, (socklen_t*)&addr_size);
        pasvMode = 1;
    }
    dataSock = d_sock;
    if (d_sock != -1) {
        sendMessage(client_sock, "Successfully Establish Connection");
    } else {
        sendMessage(client_sock, "421 PASV mode service not available, closing control connection.");
    }
}

void handleRETR(void* client_sock, struct ftp_cmd cmd) {
    if (pasvMode == 0) {
        sendMessage(client_sock, "503 Bad sequence of commands.");
        return;
    }
    if (cmd.token == NULL) {
        sendMessage(client_sock, "503 Bad sequence of commands.");
        return;
    }
    
    int inFile = open(cmd.token, O_RDONLY, 0220); 
    if (inFile == -1) {
        sendMessage(client_sock, "550 Requested action not taken. File unavailable (e.g., file not found, no access).");
        return;
    }
    char buf[512];
    int bytesRead = read(inFile, &buf, 512);
    while (bytesRead > 0) {
        send(dataSock, buf, bytesRead, 0); 
        bytesRead = read(inFile, &buf, 512);
    }
    sendMessage(client_sock, "226 Closing data connection.");
    close(inFile);
}

void handleType(char* arg ,void* client_sock) {
    if (arg == NULL) {
        sendMessage(client_sock, "503 Bad sequence of commands");
        return;
    }
    char * request = toUp(arg);
    if (arg[0] == 'A') {
        type = 'A';
        sendMessage(client_sock, "200 Type is ASCII");
    } else if (arg[0] == 'I') {
        type = 'I';
        sendMessage(client_sock, "200 Type is IMAGE");
    } else {
        sendMessage(client_sock, "504 Type command not implemented for that parameter");
    } 
}

void handleNLST(void* client_sock, struct ftp_cmd cmd) {
    if (pasvMode == 0) {
        sendMessage(client_sock, "503 Bad sequence of commands.");
        return;
    }
    sendMessage(client_sock, "125 Data connection already open; transfer starting.");
    listFiles(dataSock, ".");
    sendMessage(client_sock, "250 Requested file action okay, completed.");
}

struct ftp_cmd parseCmd(char * receive_buffer) {
    struct ftp_cmd cmd;
    char* token = toUp(strtok(receive_buffer, " "));
    cmd.cmd = token;
    int i = 0;
    cmd.token = strtok(NULL, " ");
    return cmd;
}

int cmdExists(struct ftp_cmd cmd) {
    if (strcmp(cmd.cmd, "CWD") == 0 || strcmp(cmd.cmd, "CDUP") == 0 || strcmp(cmd.cmd, "TYPE") == 0 ||
        strcmp(cmd.cmd, "MODE") == 0 || strcmp(cmd.cmd, "STRU") == 0 || strcmp(cmd.cmd, "RETR") == 0 || 
        strcmp(cmd.cmd, "PASV") == 0 || strcmp(cmd.cmd, "NLST") == 0 || strcmp(cmd.cmd, "USER") == 0){
        return 1;
    } else {
        return 0;
    }
}

// SECTION 3: Main command if/else train 

int handleCommand(void* client_sock, struct ftp_cmd cmd, char* root) {
    if (strcmp(cmd.cmd, "USER") == 0) {
        handleUser(cmd.token, client_sock);
        if (loggedIn == 1) {
            return 0;
        }
    } 
    if (strcmp(cmd.cmd, "QUIT") == 0) {
        handleQuit(client_sock);
        return 1;
    }
    if ((cmdExists(cmd) == 1) && (loggedIn == 0)) { // correct command but not logged in
        sendMessage(client_sock, "530 Not logged in.");
        return 0;
    } else if (strcmp(cmd.cmd, "CWD") == 0) { //change working  directory
        handleCWD(cmd.token, client_sock);
    } else if (strcmp(cmd.cmd, "CDUP") == 0) {
        handleCDUP(client_sock, root);
    } else if (strcmp(cmd.cmd, "TYPE") == 0) {
        handleType(cmd.token, client_sock);
    } else if (strcmp(cmd.cmd, "MODE") == 0) {
        handleMODE(cmd.token, client_sock);
    } else if (strcmp(cmd.cmd, "STRU") == 0) {
        handleSTRU(cmd.token, client_sock);
    } else if (strcmp(cmd.cmd, "RETR") == 0) {
        handleRETR(client_sock, cmd);
    } else if (strcmp(cmd.cmd, "PASV") == 0) {
        handlePASV(client_sock);
    } else if (strcmp(cmd.cmd, "NLST") == 0) { // takes in path name
        handleNLST(client_sock, cmd);
    } else {
        sendMessage(client_sock, "500 Syntax error, command unrecognized.");
    }
    return 0;
}

