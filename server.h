#include <stdio.h>
#include <stdlib.h>
#include <string.h>    //these funcs omit terminal EOF \0 etc.
#include <arpa/inet.h> // text IPv4 -> binary form.
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h> // stores all reqd. struct implementations
#include <fcntl.h>      // for open
#include <unistd.h>     // for close
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h> //for int_max debugging threads
#include <stdbool.h>

#include "DLLNode.h"
#include "packets.h"

/* Functions*/
void makedir(void);
int startserver(struct sockaddr_in serveraddr); //listening socket
char *doList();
void doExit();
bool doChatAuth(char *users, struct DLLNode *thisconn);
void doChat(char *users, struct DLLNode *thisconn);
void doUp(char *filename);
void doDown(char *filename);
void writeUserToFile(struct DLLNode *thisconn, char *filename);


/* Thread functions*/
void *s_clienthandler(void *_thisconnection);


