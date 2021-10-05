
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
//    ..... #include <sys/wait.h>

#define MAXPENDING 5
#define MAX_BUFF_LEN 255
int servSock;
int clntSock;

static pid_t child_pid;
static pid_t parent_pid;



void DieWithError(char *errorMessage) {

    perror(errorMessage);
    exit(1);
}



void store_messages(char *reciever, char *sender, char* message) {

    FILE *fp = NULL;
    char _fname[strlen(reciever) + strlen("-sms.txt") + 1];



   
    strcpy(_fname,reciever);
    strcat(_fname,"-sms.txt");

    printf("reciever== %s sender== %s, _fname== %s",reciever,sender,_fname);

    fp = fopen(_fname, "a");
    if (fp == NULL) {

        printf("Could not store message with name:%s", reciever);
        fclose(fp);
        exit(1);
    }
    //create the file
    fputs(reciever, fp);
    fputs(";", fp);
    fputs(sender, fp);
    fputs(";", fp);
    fputs(message, fp);
    fputs("\n", fp);
    fclose(fp);

}


void read_messages(char *fname){

    printf(" *** ENTERED read_messages\n");

    printf(" *** read_messages, fname== %s\n",fname);
    FILE *fp = NULL;
    char *lineptr = NULL;
    char *buffer = NULL;
    char reply [ strlen("There is no such name") +1];
    memset( reply,0, strlen("There is no such name") + 1);
    size_t n = 0;
    ssize_t bread = 0;


    strcpy(reply, "There is no such name");
    fp = fopen(fname, "r");
    if (fp == NULL){
        //no such file name
        printf(" *** read_messages, replying no such name\n");
        send(clntSock,reply, strlen(reply),0);
        return;
    }

    //file exists and can be read
    fp = fopen(fname,"r+");

    //send every message
    bread = getline(&lineptr,&n,fp);
    while(bread > -1){

        if (bread > 0) {

            printf(" *** read_messages, replying with message [%s]...\n",lineptr);
            send(clntSock, lineptr, strlen(lineptr), 0);
            bread = getline(&lineptr, &n, fp);
        }
    }
    if (lineptr == NULL){

        free(lineptr);

        printf(" *** read_messages, replying with Internal Error ...\n");
        memset(reply,0, strlen(reply));
        strcpy(reply, "Internal error");
         send( clntSock,reply, strlen(reply),0 );

        printf(" **** Error in getline\n");
        return ;
    }



    printf(" *** read_messages, replying with NO MORE MESSAGES ...\n");
    strcpy(reply, "NO MORE MESSAGES");
    send( clntSock,reply, strlen(reply),0 );

    free(lineptr);

}




void message_handler(char *buffer) {

    char *verb = NULL;
    char *token = NULL;
    char *reciever = NULL;
    char *sender = NULL;
    char *msg = NULL;
    char replying_buffer[MAX_BUFF_LEN+1];
    memset(replying_buffer,0,MAX_BUFF_LEN+1);

    printf("\n *****  message_handler ... \n");
    printf("buffer= [%s]\n",buffer);

    //convert string to lower case

    token = strtok(buffer, ";");

    int _count = 0;
    //_count < 2 for token to hold "TO"
    while (token != NULL) {

        if (_count == 0)
            verb = token;
        else if (_count == 1)
            reciever = token;
        else if (_count == 2)
            sender = token;
        else if (_count == 3)
            msg = token;

        token = strtok(NULL, ";");

        _count++;
    }


    //deal with errors
    if (verb == NULL){

        printf(" No Verb in message, reply to client...\n ");
        strcpy(replying_buffer, "No verb\n");
        send(clntSock, replying_buffer, strlen(replying_buffer), 0);
        return;
    }

    if ( reciever == NULL){

        printf(" No reciever in message, reply to client...\n ");
        strcpy(replying_buffer, "No reciever\n");
        send(clntSock, replying_buffer, strlen(replying_buffer), 0);
        return;
    }

    // --------  if verb === store then check more

    if (strcmp("store", verb) == 0) {

        printf(" verb=fetch ... check sender, message ...\n");
        if (sender == NULL) {

            printf(" Verb= fetch, no sender...\n ");
            strcpy(replying_buffer, "No sender\n");
            send(clntSock, replying_buffer, strlen(replying_buffer), 0);
            return;
        }

        if (msg == NULL) {

            printf(" Verb= fetch, no message...\n ");
            strcpy(replying_buffer, "No message\n");
            send(clntSock, replying_buffer, strlen(replying_buffer), 0);
            return;
        }

    }

    printf(" *** verb == %s, receiver=%s, sender=%s, msg=%s\n", verb, reciever,sender,msg);

    //must reply with total messages of this user
    if (strcmp("fetch", verb) == 0) {

        printf(" fetch ... replying [Retrieving messages...]...\n");
        strcpy(replying_buffer, "Retrieving messages...\n");
        send(clntSock, replying_buffer, strlen(replying_buffer),0);

        char filename[strlen(reciever) + 9];

        memset(filename,0, strlen(reciever) + 9);
        strcpy(filename,reciever);
        strcat(filename, "-sms.txt");

        printf(" *** BEFORE read_messages filename== %s\n",filename);
        read_messages(filename);

        printf(" fetch ... finished.\n");
    }
    //must write the messages
    else if ( strcmp("store", verb) == 0 ) {

        printf(" store ... replying ...\n");
        strcpy(replying_buffer, "Message accepted and stored:\n");
        send(clntSock,replying_buffer, strlen(replying_buffer),0);
        //store messages
        store_messages(reciever,sender,msg);
        printf(" store ... replyed.\n");

   }else{

        printf(" bad verb ... replying ...\n");
        strcpy(replying_buffer, "Message declined. Bad verb\n");
        send(clntSock,replying_buffer, strlen(replying_buffer),0);
        printf(" bad verb ... replyed.\n");
    }

}

void HandleTCPClient(){


    //initiating buffer
    char buffer[129], reply_buffer[129];
    memset(buffer,0,129);
    memset(reply_buffer,0,129);

    printf("\n HandleTCPClient start servicing client...\n");
    

    //accept message from client
    ssize_t bytes_read = recv(clntSock, buffer, 128, 0);

    if (bytes_read > 0 ) {

        printf("Received message : [%s]\n ", buffer);

       printf("Entering message_handler()");
        message_handler(buffer);

    }
    else
        printf("Error in receiving data from client\n");




    //shutdown client
    shutdown(clntSock,SHUT_WR);

    printf("\n HandleTCPClient ended servicing client...\n");
}




void sigintHandler(int sig_num){

    write(1,"  Caught signal... closing ports and exiting the program..\n",strlen("  Caught signal... closing ports and exiting the program..\n")+1);
    close(servSock);
    close(clntSock);
    _exit(1);

}

int main(int argc , char *argv[])
{


    printf("argc== %d\n", argc);
    if (argc != 2) DieWithError("Error: Expected server port number as argument\n");


    signal(SIGINT, sigintHandler);



    struct sockaddr_in sockServAddr;
    struct sockaddr_in sockClntAddr;
    unsigned short sockServPort;
    unsigned int clntLen;



    sockServPort = atoi(argv[1]);

    if( (servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        DieWithError("socket() failed");
    }

    int enable = 1;
    if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        DieWithError("setsockopt(SO_REUSEADDR) failed");
    }



    memset(&sockServAddr, 0, sizeof(sockServAddr));
    sockServAddr.sin_family = AF_INET;
    sockServAddr.sin_port = htons(sockServPort);
    sockServAddr.sin_addr.s_addr = htonl(INADDR_ANY);





    if (bind(servSock, (struct sockaddr *) &sockServAddr, sizeof(sockServAddr)) < 0) {
        DieWithError("bind() failed");
    }


    parent_pid = getpid();
    printf("Socket Server starts listening with pid= %d...\n",parent_pid);

    if(listen(servSock, MAXPENDING) < 0) {
        DieWithError("listen() failed");
    }



#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while(1) {


        clntLen = sizeof(sockClntAddr);
        if ((clntSock = accept(servSock, (struct sockaddr *) &sockClntAddr, &clntLen)) < 0) {
            DieWithError("accept() failed");
        }

        //fork for multiple clients
        child_pid = fork();

        if ( child_pid == 0 ){
            // we are in child now

            //close parent socket, we dont need in child
            close(servSock);

            pid_t my_pid = getpid();
            printf("Child created with pID= %d, while parent is: %d\n\n", my_pid, parent_pid);
            // here we should process clients request.....

            printf("Handling client %s\n", inet_ntoa(sockClntAddr.sin_addr));

            printf("Entering HandleTCPClient()...\n");
            HandleTCPClient() ;

            close(clntSock);

            printf(" *** child :: socket closed, exiting...\n");

            exit(0);

        }
        else if(parent_pid > 0 ) {
            printf("we are in Father. child's pid is %d\n", parent_pid);
        }
        else if (parent_pid == -1){

            printf("Could not fork()");
            close(clntSock);
            exit(1);
        }



    }


    printf("Closing sockets\n");
    close(servSock);

    return 0;
}