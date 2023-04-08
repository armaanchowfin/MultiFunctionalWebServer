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
#define MAXCLIENTS 3
#include "server.h"

pthread_mutex_t filemutex;
pthread_mutex_t exitmutex;

int num_conn = 0;

// server clienthandler thread

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Incorrect format\nUsage: ./mcfr_server Server Port\n");
        exit(-1);
    }

    unsigned short serverPort_uint = atoi(argv[1]);

    struct sockaddr_in s_serveraddr;
    memset(s_serveraddr.sin_zero, '\0', sizeof(s_serveraddr.sin_zero));
    s_serveraddr.sin_family = AF_INET;
    s_serveraddr.sin_port = htons(serverPort_uint);
    s_serveraddr.sin_addr.s_addr = htons(INADDR_ANY);

    printf("Server mode...\n");
    fflush(stdout);
    makedir();

    pthread_t s_threads[MAXCLIENTS];
    pthread_mutex_init(&filemutex, NULL);
    pthread_mutex_init(&exitmutex, NULL);

    int serverlisten_sockfd = startserver(s_serveraddr);
    printf("Server Socket fd: %d\n", serverlisten_sockfd);
    fflush(stdout);

    // pass the new accept socket as thread arg, not the listening socket.
    while (num_conn < MAXCLIENTS)
    {   
        int acceptsockfd;
        // listen on original port. Accept creates a new socket for send, recv
        if ((acceptsockfd = accept(serverlisten_sockfd, (struct sockaddr *)NULL, NULL)) < 1)
        {   
            perror("Could not accept connection, comms socket not created.\n");
            exit(2);
        }
        else
        {
            printf("Connection Accepted, fd : %d\n", acceptsockfd);
            struct DLLNode *thisconn = addUser(acceptsockfd);
            pthread_create(s_threads + num_conn, NULL, s_clienthandler, (void *)&thisconn);
            num_conn++; // oi =2 for clients = 2.
        }
    }

    for (int i = 0; i < MAXCLIENTS; i++)
    {
        pthread_join(s_threads[i], NULL);
    }

    pthread_mutex_destroy(&filemutex);
    pthread_mutex_destroy(&exitmutex);
}

/*Tasks:
1. send server responses
2. Chat functions.
*/

void *s_clienthandler(void *_thisconn){
    struct DLLNode **thisconn_p = (struct DLLNode**)_thisconn;
    struct DLLNode *thisconn = *thisconn_p;
    int mysockfd = thisconn->state.s_sockfd; //filled during accept.

    // pid_t mytid = (thisconn->threadhandler = gettid());
    // printf("Connection accepted, handled by thread %d...\n", thisconn->threadhandler);
    
    
    
    char initmsg[1024] = "Enter username:";
    int sent = send(mysockfd, initmsg, sizeof(initmsg), 0);


    //puts("here");
    if(sent == -1 && errno !=EINTR){
        perror("send fail\n");
        exit(EXIT_FAILURE);
    }

    struct packet *recstruct = malloc(sizeof(struct packet));
    int recd = recpacket(recstruct, mysockfd);
    char uname[1024] = {0};
    strcpy(uname,recstruct->username); // populate recstruct via constrained memcpy. then use it to populate DLLNode
    strcpy(thisconn->state.clientname, uname); //DLLNode updated.

    printf("DLLNode: %s\n", thisconn->state.clientname);

    writeUserToFile(thisconn, "user.txt"); //write username@IP.

    free(recstruct);

    while(1){
        struct packet *recpack = malloc(sizeof(struct packet));
        recpacket(recpack, mysockfd); //populates recpkt fields.
        enum MessageType mtype = recpack->mtype; //
        char *args = recpack->msg_args; //Client list.

        switch(mtype){

        case LIST:
        //mutex
            //doList();
            break;

        case EXIT:
        //mutex
            pthread_mutex_lock(&exitmutex);
            doExit(thisconn);
            pthread_mutex_unlock(&exitmutex);
            break;

        case CHAT:
            if(doChatAuth(args, thisconn)) doChat(args, thisconn); //blocks the requesting client only.
            break;

        //currently only client-> server, later client-client via server. will need auth etc.
        case UP: 
           // doUp(args);
            break;

        case DOWN:
           // doDown(args);
            break;
        }
    }
}

//make better.
void writeUserToFile(struct DLLNode *thisconn, char *filename){
   //create user@IP
    char tofile[2048] = {0};//username@IP\n
    char tolist[2048] = {0}; //username@IP

    int mysockfd = thisconn->state.s_sockfd;
    //get client IP
    struct sockaddr_in clientaddr; // stores IP
    char s_clientIP[INET_ADDRSTRLEN]; 
    socklen_t clientaddr_len = sizeof(clientaddr);

// clientaddr from TCP header.
    getsockname(mysockfd, (struct sockaddr *)&clientaddr, &clientaddr_len); //returns OS- assigned client port and IP in to clientaddr struct.
    inet_ntop(AF_INET, &clientaddr.sin_addr, s_clientIP, sizeof(s_clientIP));// store IP 

    //concat strings
    char *username = thisconn->state.clientname;

    snprintf(tolist, sizeof(tolist), "%s@%s", username, s_clientIP);
    snprintf(tofile, sizeof(tofile), "%s@%s\n", username, s_clientIP);

    //write user@IP to DLL node (overwrite)
    strcpy(thisconn->state.clientname, tolist); //no \n here.

    //write user@IP to file
    pthread_mutex_lock(&filemutex);
    FILE *fptr = fopen(filename, "a+");
    fputs(tofile, fptr); //write occurs before send from clientside.
    fclose(fptr);
    pthread_mutex_unlock(&filemutex);

}
void doUP(char *filename){
    ;
}

void doDown(char *filename){
    ;
}

char *doList(void){
    //struct DLLNode *head global.
//     struct DLLNode *current = head;
//     while(current !=NULL){
        
//     }
        
}
void doExit(struct DLLNode *thisconn){

    //delete item from file.

    char *line = NULL;
    char users[MAXCLIENTS][1024];
    ssize_t len = 0;

    FILE *fptr = fopen("user.txt", "r");
    
    //store lines in buffer.
    int i = 0;//i <MAXCLIENTS
    while ((getline(&line, &len, fptr)) != -1){ 
        strcpy(users[i], line);
        i++;//linecount
    }
    fclose(fptr); 

    //write only reqd user to new file.
    FILE *fptrx = fopen("user.txt", "w");
    for(int j = 0; j<i; j++){
        if(strcmp(users[j], thisconn->state.clientname)){
            fputs(users[j],fptrx);
        }
    }
    fclose(fptrx);
    
    //delete DLL node
    removeUserByName(thisconn->state.clientname); //this works :)
    num_conn--;
}

bool doChatAuth(char *args, struct DLLNode *thisconn){
    int mysockfd = thisconn->state.s_sockfd; //server to thisclient thread.
    char thisusername[1024] = {0};
    strcpy(thisusername, thisconn->state.clientname);

    bool isauth = false;
    /*
    inspired by telnet.
    1. get list of chat client sockfds from DLL.
    2. make and send DO packet (CHAT messagepacket with DO bit true). Just edit same struct
    3. rec a WILL CHAT if client inchat false, else rec a WONT CHAT (can switch on DO/WILL flag only?)
    4. if WILL: return true + update clientstate inchat on server.
    */
   char *user;
   while ((user = strsep(&args, " "))){
    struct packet authpack = makeauthpkt(CHAT, DO); //DO
    authpack.isChatAuth = false;
    struct DLLNode *chatclient = malloc(sizeof(struct DLLNode));
    chatclient = findUserByName(user);
    if(chatclient == NULL){
        printf("%s Not Connected\n", user);
        return isauth;
    }
    else{
        int thisclientsockfd = chatclient->state.s_sockfd;

        //send DO
        sendpacket(authpack, thisclientsockfd);

        recpacket(&authpack, thisclientsockfd);
        //server never recs DO.
        char authmessage[1024] = {0};

        switch(authpack.authtype){
            case WILL:
                //chatclient can chat!!
                sprintf(authmessage, "CONN Client %s : Y", chatclient->state.clientname); //chat client name
                strcpy(authpack.msg_args, authmessage); 
                sendpacket(authpack, mysockfd); //WILL pack sent to client.
                isauth = true;
                break;

            case WONT:
                sprintf(authmessage, "CONN Client %s : N", chatclient->state.clientname); //chat client name
                strcpy(authpack.msg_args, authmessage); 
                sendpacket(authpack, mysockfd); //WILL pack sent to client.
                break;
            }

        }
    }
    return isauth;
}

void doChat(char *users, struct DLLNode *thisconn){
    int mysockfd = thisconn->state.s_sockfd; //server to thisclient thread.
    char thisusername[1024] = {0};
    strcpy(thisusername, thisconn->state.clientname);
   
    /*
    while(1) 
    1. recmessagepack from myclient process
    2. printmessage on server terminal
    3. save message to chat log
    4. sendmessagepacket to chatclient process(es)
    */
   while(1){
        struct packet messagepack = {0};
        recpacket(&messagepack, mysockfd); // from client clientinputhandler thread. rec/send socket for client handled by the thread.
        
        if(!strcmp(messagepack.msg_args, "exit:")) return; //goes back to listening for mesg.

        //if(messagepack.mtype == NONE){//regular message.
        printf("%s\n", messagepack.msg_args); //message from a client on server terminal.
        struct DLLNode *chatclient = findUserByName(users); //single arg for now.
        int chatsockfd = chatclient->state.s_sockfd;

        char chatmessage[2050] = {0};
        sprintf(chatmessage, "%s : %s", thisusername, messagepack.msg_args);
        sendpacket(messagepack, chatsockfd);
   }

}



void makedir(void){
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("CWD: %s\n", cwd);

    errno = 0;
    char *server_dir_path  = strcat(cwd,"/ServerWD/");
    int dir_result = mkdir(server_dir_path, 0755);
    
    if(dir_result != 0 && errno == EEXIST){
        printf("Server Directory already exists.\n");
        chdir(server_dir_path);
        printf("Server New CWD: %s\n", server_dir_path);   
    }
    else{
        if(chdir(server_dir_path) == 0){
        printf("Server New CWD: %s\n", server_dir_path);
        }
    }
    FILE *fh = fopen("user.txt", "w");
    fclose(fh);
}

int startserver(struct sockaddr_in s_serveraddr){
    //create server socket
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0); //socket creates fd i.e. index of fd table that contains pointer to a file on local machine, on which messages are read/written.
    if(server_sockfd < 0){
        perror("Socket Creation failed.");
        exit(-1);
    }

    //bind socket to server addr ONLY. Server socket created at port + addr.
    socklen_t s_serveraddr_len = sizeof(s_serveraddr);
    if(bind(server_sockfd, (struct sockaddr *)&s_serveraddr/*struct ptr to struct ptr*/, s_serveraddr_len) < 0){
        perror("Server socket bind failed.");
        exit(-2);
    }
    
    if(listen(server_sockfd, 5)!= 0){
        perror("Listening server socket not created...\n");
        exit(-3);
    } //socket @sockfd passively waits for connect req from client.

    return server_sockfd;
}

