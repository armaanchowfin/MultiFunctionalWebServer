#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


enum MessageType {NONE = 0, LIST = 1, EXIT = 2, CHAT = 3, UP = 4, DOWN = 5};
enum AuthType {DO = 0, WILL = 1, WONT = 2};
enum PacketType {MESSAGE = 0, AUTH = 1, INIT = 2};

//could eventually combine the packets into a single one, with a 2bit identifier.
struct packet{ //2064 bytes.
    enum MessageType mtype; // CHAT LIST etc
    enum PacketType ptype; // MESSGAE, AUTH
    enum AuthType authtype; //DO, WILL,WONT
    char username[1024];
    char msg_args[1024];
    bool F_Do;
    bool F_Will;    
    bool F_Wont;
    bool isChatAuth; //set after auth done. Can eliminate initpacket type. switch NONE, if ischatauth
}__attribute__((__packed__));

enum PacketType str_toenum(char *type);

struct packet makeinitpkt(char *name);
struct packet makeauthpkt(int type, int subtype);
struct packet makemessagepkt(char *raw_input);

int sendpacket(struct packet thispkt, int sockfd);
int recpacket(struct packet *thispkt, int sockfd); //recs a bytestream into internal buf. memcpy to struct addr defined outside 




