/*
1. threads implicitly share memory via their heap.
-------CLIENT SIDE-------
When client connected, one thread created that haas a recv in a while loop to handle server messages
another thread created that has fgets in a while loop to handle client input.
// clientsockfd is local to client mode func
- WHen the server sends a message to clientsockfd, it is recv'd by the recv thread.
-fgets thread is running simultaneously the whole time. If cmd line input entered from client,
all the string processing (EXIT/LIST), username etc are stored and sent to serversockfd from client.
- Sync issues on client side?

-------SERVER SIDE-------
- does server take cmd line inputs?NO
- Server spawns a thread anytime an accept is done. Each thread handles one client.
- Server behaviour remains the same for all clients, hence the same thread function can be called.
- this clienthandler thread function remains the same as earlier. does both rec and send.
- perform actions based on client input string.

--------ADDITIONAL FUNCTIONALITIES--------
- Large File download from server.
- CONN user@IP: Initiate 2-client chat, extend to broadcast to m clients.
- UPLOAD, DOWNLOAD. filenames in "file.txt".
*/

#define _GNU_SOURCE
#define NUMCLIENTS 3
#include "client.h"

//can be populated from anywhere.

void *c_chathandler(void *_thisclientstate);

int is_username_prompt_printed = 0;

pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; 

int main(int argc, char *argv[]){
    if(argc != 3){
    printf("Incorrect format\nUsage: ./mcc_server ServerIP Server Port\n");
    exit(-1);
    }

    char *s_IP = argv[1];
    unsigned short s_Port = atoi(argv[2]);

    struct sockaddr_in c_serveraddr;
    memset(c_serveraddr.sin_zero, '\0', sizeof(c_serveraddr.sin_zero));
    c_serveraddr.sin_family = AF_INET;
    c_serveraddr.sin_port = htons(s_Port);
    if(inet_pton(AF_INET, s_IP, &c_serveraddr.sin_addr) < 1){
       perror("Invalid IP Address.\n");
    }

    //create client socket.
    int mysockfd = (thisclientstate.sockfd = socket(AF_INET, SOCK_STREAM, 0));
    printf("client sockfd: %d\n", thisclientstate.sockfd);
    
    //connect client socket to server, 
    socklen_t c_serveraddrlen = sizeof(c_serveraddr);
    if(connect(mysockfd, (struct sockaddr *)&c_serveraddr, c_serveraddrlen) < 0){
        perror("Connect failed.");
        exit(-2);
    }
    else{
        thisclientstate.inchat = false;

        print_connectionpair(thisclientstate.sockfd, c_serveraddr);

        pthread_t th_clientinput, th_servermessage;

        pthread_mutex_init(&mutex2,NULL);
        //random thread order could cause bug.
        pthread_create(&th_servermessage, NULL, c_servermessagehandler, (void *)NULL);
        pthread_create(&th_clientinput, NULL, c_clientinputhandler, (void *)NULL);
        
        pthread_join(th_servermessage, NULL);
        pthread_join(th_clientinput,NULL);

        pthread_mutex_destroy(&mutex2);

    }

}

//needs to run AFTER username recd thread.
void *c_clientinputhandler(void *){

    //wait for serrverresponse thread signal
    pthread_mutex_lock(&mutex2);
    while (!is_username_prompt_printed){
        pthread_cond_wait(&cond2, &mutex2);
    }
    pthread_mutex_unlock(&mutex2);
    
    int mysockfd = thisclientstate.sockfd;
    char rawinput[1024] = {0};

    fgets(rawinput, sizeof(rawinput), stdin);
    rawinput[strcspn(rawinput, "\n")] = 0;
    strcpy(thisclientstate.username, rawinput);

    struct packet thisinitpkt = makeinitpkt(rawinput);
    int sent = sendpacket(thisinitpkt, mysockfd); //need to send full packet not just 8 bytes.

//client always here.
    while(1){ //CHAT armaan or LIST\n or EXIT or DOWN filename.txt or hi sup etc.s
        memset(rawinput, 0, sizeof(rawinput));
        fgets(rawinput, sizeof(rawinput), stdin);
        //rawinput[strcspn(rawinput, "\n")] = 0;
        struct packet thismsgpkt = makemessagepkt(rawinput);//message type filled
        thismsgpkt.isChatAuth = thisclientstate.inchat;
        sendpacket(thismsgpkt, mysockfd);
    } 
}

//needs to be listening at recv at all times, even when c_chathandler is running.
//handles even chat received.
void *c_servermessagehandler(void *){
    //puts("servermessage\n");
    pthread_mutex_lock(&mutex2);
    int mysockfd = thisclientstate.sockfd;
    bool mychatstate = thisclientstate.inchat;

    char initmsg[1024] = {0};

    //rec enter username
    int recd = recv(mysockfd, initmsg, sizeof(initmsg), 0);
    printf("%s\n", initmsg);
    is_username_prompt_printed = 1;
    pthread_cond_signal(&cond2);

    pthread_mutex_unlock(&mutex2);

//for every client. some client could be in recv block.
        /*
        1. rec a packet 
        2. if message: print sender: message
        2b. if auth:  check state.inchat, update packet, send packet.
        */    
    while(1){
        struct packet thisrecpacket = {0};
        //recpacket not working.
        int recd = recpacket(&thisrecpacket, mysockfd);
        printf("recpacket message args: %s\n", thisrecpacket.msg_args);

        switch(thisrecpacket.ptype){
            case AUTH:
                switch(thisrecpacket.authtype){
                    case DO:
                        if(thisclientstate.inchat) {
                            thisrecpacket.isChatAuth = false;
                            thisrecpacket.authtype = WONT;
                            sendpacket(thisrecpacket, mysockfd);
                        }
                        else{
                            thisrecpacket.isChatAuth = true;
                            thisrecpacket.authtype = WILL;
                            sendpacket(thisrecpacket, mysockfd);
                        }
                        break;

                    case WILL: //auth done, recd by sender client at top line, switch again.
                        printf("%s\n", thisrecpacket.msg_args); //CONN Client2: Y
                        thisclientstate.inchat = true;
                        break;

                    case WONT:
                        printf("%s\n", thisrecpacket.msg_args); //CONN Client2: N
                        thisclientstate.inchat = false;
                        break;

                }
                break;

            case MESSAGE:
            //print the args with sender client name.
            puts("message\n");
            char chatmessage[1024] = {0}; //recs senderclient : Message.
            strcpy(chatmessage, thisrecpacket.msg_args);
            printf("%s\n", chatmessage);                
            }
        }


        //puts("waiting for serevr msg");





        // if(!strcmp(mesgid, "MESG")){//recd MESG Enter Username
        //     printf("%s", message);
        //     fflush(stdout);
            
        //     // fix a synchro bug.
        //     pthread_mutex_lock(&mutex2);
        //     if(!pthread_cond_wait(&cond2, &mutex2)){
        //         strcpy(message, thisclientstate.username);
        //         message[strcspn(message, "\n")] = 0;
        //         printf("my username: %s\n", message);//no space EOF, pure.
        //         send(thisclientstate.sockfd, message, sizeof(message),0);
        //     }
        //     pthread_mutex_unlock(&mutex2);
        // }
        // else if(!strcmp(mesgid,"CURR")){
        //     printf("%s\n", message);
        // }
        // else if(!strcmp(mesgid, "CONN")){
        //     puts("here");
        //     printf("Message: %s\n", message);
        //     char availablemesg[1024] = {0};
        //     if(!thisclientstate.inchat){
        //         puts("jere");
        //         sprintf(availablemesg, "CONN %s : Y", message);//message sent from server is CONN Client3
        //         send(thisclientstate_sockfd, availablemesg,sizeof(availablemesg), 0);
                
        //         pthread_t thclientchat;
        //         if(!strcmp(message,"true")){
        //             puts("canchat");
        //             pthread_create(&thclientchat, NULL, c_chathandler, (void *)&thisclientstate);
        //         }
        //         else{
        //             printf("Cannot chat\n");
        //         }
        //     }
        //     else{
        //         puts("nothere");
        //         sprintf(availablemesg, "CONN %s : N", message);
        //         send(thisclientstate_sockfd, availablemesg,sizeof(availablemesg), 0);
        //     }

        // }

    }



void print_connectionpair(int client_sockfd, struct sockaddr_in c_serveraddr){
    struct sockaddr_in c_clientaddr;
    socklen_t c_clientaddr_len = sizeof(c_clientaddr);
    bzero(&c_clientaddr, c_clientaddr_len);

    // use getsockname(client_sockfd) here post connect to server.
    getsockname(client_sockfd, (struct sockaddr *)&c_clientaddr, &c_clientaddr_len); //returns OS- assigned client port and IP in to clientaddr struct.

    char c_client_IP[INET_ADDRSTRLEN], c_server_IP[INET_ADDRSTRLEN]; //buffers to store IPaddrs

    inet_ntop(AF_INET, &c_clientaddr.sin_addr, c_client_IP, sizeof(c_client_IP)); // client sin_addr assigned in client mode by OS when connect made.
    inet_ntop(AF_INET, &c_serveraddr.sin_addr, c_server_IP, sizeof(c_server_IP)); // server sin_addr assigned in main

    printf("Client connected to server.\nClient has both Client Socket %s:%u and Server Socket %s:%u information\n", c_client_IP, ntohs(c_clientaddr.sin_port), c_server_IP, ntohs(c_serveraddr.sin_port));// will be different for each client
}