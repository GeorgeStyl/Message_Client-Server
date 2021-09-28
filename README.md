# Message_Client-Server
Messaging Client - Server application

## Message Socket Server (server.c)
- Uses TCP/IP (stream socket)
- Requieres:
  1. 1 command parameter
  2. The port number to bind to
- Binds on **ANY** IP address of it's host (and on local host, 127.0.0.1)
- Uses fork() in order to serve multiple clients stimuntaniously 
- Uses a buffer of 255 bytes (global macro #define MAX_BUFF_LEN 255)
- Traps **Ctr+C** from console so that before the server terminates, the sockets       will be closed
- The protocol which is used on Application level for the connection  between server and client is the following:
>VERB;reciepient[;sender;message] (comma delimited line)  
wheres:
 - If verb is "fetch", it means: "fetch every message for the following reciepient"
 - If verb is "strore", it means: "store the message with reciepient, sender, message"  
  So, when the client (for example: with username == george) wants to send a message to user Kyriakos, he will have to send the following string: "store;Kyriakos;george;hi Kyriakos"  
    
    
  >Also, when the client (for example username == george) wants to see his messages, he has to type the following string: fetch;george  

## Server Operation
- When the server binds and listens on IP, it waits theconnection of a client
- When a client connects, the server must open a socket again with the server and send it's message. After serving the client, server's socket close  
  
By doing so, server doesn't get on blocking mode if a client fails or be late to inform that it's messages have finished.


## Message client (cleint.c)
- Uses TCP/IP (stream socket)
- Requires:
  1. IP address of the server
  2. The number of port that the server will listen to
- Has a menu with 3 operations such as:
  1. Send message
  2. Read message
  3. Exit


## Client operation
According to the server's mode of operation and the predefined client's protocol, for each request to the server creates a socket ( connect() ), sends the request ( send() ) and takes the answer. Then the client closes the socket and returns to menu.

  

