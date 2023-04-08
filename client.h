#include <netinet/in.h> //sockadrr_in
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //these funcs omit terminal EOF \0 etc.
#include <arpa/inet.h> // text IPv4 -> binary form.
#include <sys/types.h>
#include <sys/socket.h> // stores all reqd. struct implementations
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h> //for int_max debugging thread
#include <time.h>
#include <stdbool.h>

#include "packets.h"

//LOCAL TO CLIENT. STRUCT NEVER SENT. ONLY PACKETS SENT.
typedef struct ClientState{
    char username[100]; 
    bool inchat;
    int sockfd;  //only used from client process.
    struct sockaddr_in clientaddr;
}__attribute__((packed, aligned(1))) ClientState;

ClientState thisclientstate; 

void print_connectionpair(int client_sockfd, struct sockaddr_in c_serveraddr);

void *c_clientinputhandler(void *); //arg clientsockfd.
void *c_servermessagehandler(void *);
void print_connectionpair(int client_sockfd, struct sockaddr_in c_serveraddr);