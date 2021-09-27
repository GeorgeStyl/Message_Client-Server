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
- Traps **Ctr+C** from console so that before the server terminates, the sockets will be closed
- The protocol which is used on Application level for the connection  between server and client is the following:
>VERB;reciepient[;sender;message] (comma delimited line)
Wheres:
- If verb is "fetch", it means: "fetch every message for the following reciepient"
- If verb is "strore", it means: "store the message with reciepient, sender, message"

