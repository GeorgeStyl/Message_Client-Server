
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>	    /* for memset() strlen  */
#include <sys/socket.h> /* for socket(), bind(), connect(), recv() and send() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_ntoa() inet_addr */ 
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>     /* for close() */
#include <sys/time.h>

#include <errno.h>

#include <signal.h>    /* for signals */
#include <sys/wait.h> /* for waitpid() */


#define PORT 5129
#define DEF_PROTO  SOCK_STREAM
#define GET_SMS_VERB "fetch"
#define SEND_SMS_VERB "store"
#define RECEIVEBUFFSIZE 128
#define BUFFER_SIZE 1024


void DieWithError(char *errorMessage) {

    perror(errorMessage);
    exit(1);
}



void sendSms( char * serveridaddress, unsigned short port, char * username ){

    printf(" ** sendSMS called...\n");

    //establish communication with the server
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(serveridaddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );



    int s ;
    int socket_type = DEF_PROTO;
    if((s = socket(AF_INET , socket_type , 0 )) < 0 )
    {
        printf(" sendSms: ERROR Could not create socket.");
        return ;
    }
    printf(" sendSms: Socket created.\n");

    printf(" sendSms: Connecting to sms server...");
    if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf(" sendSms: connect error");
        return;
    }
    printf(" sendSms: Connected. \n");



    //read stdin
    char *buffer = NULL;
    char *to =NULL ;
    char message[255];

    //catch previous enter
   getchar();

    printf(" Type the receipient (50chars max): ");
    size_t n = 0;
    n = getline(&to,&n,stdin);
    if(n<1) {
        printf(" failed to read line for receipient !");
        return;
    }
    to[strlen(to)-1]=0; // clear LF at the end

    //get message
    printf(" Type your sms: ");
    n = 0;
    n = getline(&buffer,&n,stdin);
    if(n<1) {
        printf(" failed to read line for sms !");
        return;
    }
    buffer[strlen(buffer)-1]=0; // clear LF at the end

    if (n >= BUFFER_SIZE - strlen(buffer) ) {
        printf(" Message too long\n");
        return;
    }

    //create the message to send to server
    sprintf(message, "%s;%s;%s;%s",SEND_SMS_VERB,to,username,buffer);
    printf(" sendSms: sending message [%s] \n", message);


    //Receive a reply from the server
    char server_reply[8000];
    memset(server_reply,0,8000);

    char recvbuff[RECEIVEBUFFSIZE+1];
    memset(recvbuff,0,RECEIVEBUFFSIZE+1);


    if( send(s , message , strlen(message) , 0) < 0)		// == SOCKET_ERROR
    {
        printf(" sendSms: Send failed");
        return ;
    }
    printf(" sendSms: Sms sent succesfully.\n");

    printf("============================================================= receiving reply ...\n");
    int recv_size;
    while( (recv_size = recv(s , recvbuff , RECEIVEBUFFSIZE , 0)) > 0 )
        // if((recv_size = recv(s , server_reply , 8000 , 0)) == SOCKET_ERROR)
    {
        //Add a NULL terminating character to make it a proper string before printing
        recvbuff[recv_size] = '\0';
        printf(" receiveSms: received %d bytes:[%s]\n", recv_size, recvbuff);
        strcat(server_reply, recvbuff);
        // ??? if( stricmp(verb,  "sendsms") == 0 ) break;
    }
    printf("============================ reception completed.\n");
    printf(" sendSms: Total server reply:\n");
    printf("%s\n",server_reply);
    // close socket and cleanup
    printf(" sendSms: Closing socket and exiting... \n");
    close(s);
    printf("============================ \n socket closed. End\n");

    free(buffer);
    return;
}


void receiveSms( char * serveridaddress, unsigned short port, char * username ){

    printf(" ** receiveSMS called...\n");
    struct sockaddr_in server;
    // SOCKET s = INVALID_SOCKET;
    server.sin_addr.s_addr = inet_addr(serveridaddress);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );

    int socket_type = DEF_PROTO;
    int iResult;
    char message[255];

    int s ;
    if((s = socket(AF_INET , socket_type , 0 )) < 0 )
    {
        printf(" receiveSms: ERROR Could not create socket.");
        return ;
    }
    printf(" receiveSms: Socket created.\n");

    //Connect to remote server
    printf(" receiveSms: Connecting to sms server...");
    if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf(" receiveSms: connect error");
        return;
    }
    printf(" receiveSms: Connected. \n");

    sprintf(message, "%s;%s", GET_SMS_VERB, username );
    printf(" receiveSms: sending message [%s] \n", message);

    //Receive a reply from the server
    char server_reply[8000];
    memset(server_reply,0,8000);

    char recvbuff[RECEIVEBUFFSIZE+1];
    memset(recvbuff,0,RECEIVEBUFFSIZE+1);


    if( send(s , message , strlen(message) , 0) < 0)		// == SOCKET_ERROR
    {
        printf(" receiveSms: Send failed");
        return ;
    }
    printf(" receiveSms: Data Send.\n");

    printf("============================================================= receiving ...\n");
    int recv_size;
    while( (recv_size = recv(s , recvbuff , RECEIVEBUFFSIZE , 0)) > 0 )
    {
        //Add a NULL terminating character to make it a proper string before printing
        recvbuff[recv_size] = '\0';
        printf(" receiveSms: received %d bytes:[%s]\n", recv_size, recvbuff);
        strcat(server_reply, recvbuff);
    }
    
    printf("============================================================= reception completed.\n");
    printf(" receiveSms: Total server reply:\n");
    printf("%s\n",server_reply);
    // close socket and cleanup
    printf(" receiveSms: Closing socket and exiting... \n");
    close(s);
    printf("=============================================================\n socket closed. End\n");

    return;
}
  

int main(int argc , char *argv[])
{

    char username[50];
    char * serveridaddress;
    char * serverport;
    unsigned short port = PORT ;

    if( argc < 3) {
        printf("\n Bad client execution parameters !\n");
        printf(" Usage: ./client server_ip_address server_port \n");
        printf(" Exiting\n");
        return(1);
    }

    serveridaddress = argv[1];
    serverport = argv[2];
    port = atoi(argv[2]);

    memset(username,0,50);
    printf("\n ===== sms Socket Client ===== \n");
    printf(" by Georgios Stylianopoulos A.M.: 2022201900219\n\n");
    printf(" Enter username to continue (up to 50 chars):");
    scanf(" %s", username);

    int menuchoice=0;
    while(menuchoice!=3) {
        printf("\n ===== sms Socket Client ===== \n");
        printf(" using serverer %s:%d, username:%s \n",serveridaddress,port,username);
        printf(" Choices are:\n");
        printf(" 1. Send sms \n");
        printf(" 2. Receive smses \n");
        printf(" 3. Quit \n");
        printf(" Enter your choice: ");
        scanf(" %d", &menuchoice);


        if(menuchoice==3) break;


        if(menuchoice==1)  sendSms( serveridaddress,port,username );


        if(menuchoice==2)  receiveSms( serveridaddress,port,username );
        


    }
    printf("\n sms Client Program Finished.\n");
    return 0;
}


