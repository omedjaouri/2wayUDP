#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <iostream>
#include <pthread.h>
#include "_fifo_queue.h"
#include "strings.h"
#include "string.h"


#define MAX_RECV_LENGTH 512
#define MAX_SEND_LENGTH 512
#define MAX_BUFFER_LENGTH 4086
#define MAX_MESSAGES 12


/*

   FOR LIBRARY USE, NOT NEEDED BY USER
   
   Shifts an array
   
   Inputs:
      int* array = array to be shifted
      int shift = the amount to be shifted
      int init_val = index to be shifted from
   
*/
void array_shift(int* array, int shift, int init_val);

/*

   FOR LIBRARY USE, NOT NEEDED BY USER
   
   Initializes a socket to listen to for incoming messages
   
   Inputs:
      int sockfd = uninitialized integer
      int type = AF_INET (Iv4)
      int domain = SOCK_DGRAM, SOCK_STREAM, etc.
      int protocol = 0
      int portno = port number of the socket
   Outputs:
      Returns the id of a socket

*/
int socketInit(int sockfd, int type, int domain, int protocol,
               int portno);

/*

   FOR LIBRARY USE, NOT NEEDED BY USER

   Initializes a sockaddr variable with information about destination
   
   Inputs:
      int type = AF_INET(Iv4)
      int portno = port number of the socket at destination
      char* IPaddr = Destination IP address

*/
void clientInit(int type, int portno, char* IPaddr);

/*

   FOR LIBRARY USE, NOT NEEDED BY USER

   Wrapper for the sendto function. It reads messages from the queue
   and sends them. Runs as a background process so that the user can
   add messages to the queue at any point and have them sent while
   the user processes data.

*/
void *Send(void);

/*

   FOR LIBRARY USE, NOT NEEDED BY USER

   Wrapper for the recvfrom function. Runs as a background process.
   Whenever it receives a message, it places it on a queue that the
   user can access via userRead. When the queue fills, it flushes
   out the queue so the user must keep in mind how large the queue
   is.

*/
void *Receive(void);

/*

   FOR USE BY THE USER
   
   Allows the user to pass a message to the queue to be sent. After
   the function returns, the user does not need to wait for the 
   message to be sent because it is being sent in a background 
   process.

   Inputs:
      char* message =  the message to put on queue
   Outputs:
      Returns number of bytes put on queue or -1 if the queue was full

*/
int userWrite(char* message);

/*

   FOR USE BY THE USER

   Allows the user to read a message from the queue written to by
   the recvfrom wrapper function. If the queue is empty, the 
   function will return a -1, otherwise the function will return
   the number of bytes long the message is.
   
   Inputs:
      char* buffer = buffer for the function to write the message
   Ouputs:
      Returns -1 if no messages, otherwise returns number of bytes
      in the message.

*/
int userRead(char* buffer);

/*

   FOR USE BY THE USER

   Initializes the library by creating the socket for the library
   to listen on as well as taking data like destination address
   and port and making that information usable by library functions.
   Also creates threads for the Send and Receive functions so that
   they can run in the background while the user uses userRead/Write
   to passes messages to queues.

   Inputs:
      char* IPaddr = Destination IP address
      int destination_portnumber = Port number of destination
      int portno = Port number the library will listen on.

*/
void commInit(char* IPaddr, int dest_portnumber, int portno);
