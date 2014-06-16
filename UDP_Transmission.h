#ifndef UDP_TRANSMISSION_H
#define UDP_TRANSMISSION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <iostream>
#include "_fifo_queue.h"

#define MAX_RECV_LENGTH 512
#define MAX_SEND_LENGTH 512
#define MAX_BUFFER_LENGTH 4086
#define MAX_MESSAGES 8

extern struct sockaddr_in me_addr, other_addr;
extern char recv_data[MAX_RECV_LENGTH], send_data[MAX_SEND_LENGTH];
extern int inbuffer[MAX_MESSAGES], outbuffer[MAX_MESSAGES];
extern int sock;
extern struct _queue* inqueue, outqueue;
/*
   ** FOR USE BY LIBRARY, DOES NOT NEED TO BE CALLED DIRECTLY **
   
   Shifts array left a given amount of steps.
*/
void array_shift(int* array, int shift, int init_val);
/*
  ** FOR USE BY LIBRARY, DOES NOT NEED TO BE CALLED DIRECTLY **

   Initializes a socket with given ID and returns the socketID
   as an integer.
      

   Inputs:
      int type: type of communication protocol to use (AF_INET = Iv4)
      int domain: message type (SOCK_DGRAM for UDP)
      int protocol: leave as 0 for UDP
   Output:
      Returns socket ID
      
*/
int socketInit(int sockfd, int type, int domain, int protocol,
               int portno);
/*
   ** FOR USE BY LIBRARY, DOES NOT NEED TO BE CALLED DIRECTLY **
      
   Initializes destination address to communicate to


   Inputs:
      int type: type of communication protocol to use (AF_INET = Iv4)
      int portno: Port number of the destination
      char* IPaddr: Iv4 IP address of the destination
*/
void clientInit(int type, int portno, char* IPaddr);
/*
   ** FOR USE BY LIBRARY, DOES NOT NEED TO BE CALLED DIRECTLY **

   Wrapper for sendto function
   
   
   Inputs:
      int sockid = SockID
      char* buffer = Array with intended message
      int buflen = max length of buffer
*/
void Send(int sockid, char* buffer, int buflen);

/*
   ** FOR USE BY LIBRARY, DOES NOT NEED TO BE CALLED DIRECTLY **

   Wrapper for recvfrom function


   Inputs:
      int sockid = SockID
      char* buffer = array to store received data
      int buflen = max length of the buffer
   Outputs:
      Returns the number of bits received
*/
int Receive(int sockid, char* buffer, int buflen);

/*
   Takes User message and passes it onto the queue so that it is
   sent as soon as possible


   Inputs:
      char* message = Null terminated string
*/
bool userWrite(char* message);      

/*
   Takes data from the queue and returns it to the User
   
   
   Outputs: 
      Returns buffer for use by the user
*/
bool userRead(char* buffer);

/*
   Initializes the communication. Creates two separate processes,
   one for receiving messages and one for sending messages.
   User passes messages to the send queue so that the sending
   process can send the messages as soon as possible. For the 
   user to receive messages, they must call userRead to pull 
   messages from the queue.

   
   Inputs:
      char* IPaddr = IP address of the destination
      int portnumber = Port number of the destination
*/
void communicationInit(char* IPaddr, int dest_portnumber, 
                       int portno, int ticks);

#endif
