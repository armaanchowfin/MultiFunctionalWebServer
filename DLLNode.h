//client side payload struct to be loaded into DLLNode.
#ifndef DLLNode_H_
#define DLLNode_H_
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#define MAXCLIENTS 3

struct ClientState{
    char clientname[100]; //filled after recv initpacket
    bool inchat; //modified by doAuth.
    int s_sockfd; // filled by server after accept
    struct sockaddr_in clientaddr; //send from client.
}__attribute__((packed, aligned(1)));

struct DLLNode{
    struct DLLNode *prev;
    struct ClientState state;
    struct DLLNode *next;
};

/*DLL Functions*/
struct DLLNode *createUser(int sockfd);
struct DLLNode *addUser(int sockfd);
struct DLLNode *findUserByName(char *name);
char *getDLLNodes(void);
void removeUserByName(char *name);

#endif