#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <iostream>

#define MAX_RECV_LENGTH  1024
#define MAX_SEND_LENGTH  1024

//Two-Way communication only
struct sockaddr_in me_addr, other_addr;
char recv_data[MAX_RECV_LENGTH], send_data[MAX_SEND_LENGTH];

/*
   Initializes a socket with given id and returns the socketID
   as an integer. 
   Inputs:
      type = 
      domain = message type i.e. SOCK_DGRAM, SOCK_STREAM
      protocol = 0 for UDP
   Output:
      Returns sockID
*/

int socketInit(int sockfd, int type, int domain, int protocol, 
               int portno){
   
   //Make Socket (Iv4, Fixed message length)
   if((sockfd = socket(type, domain, protocol)) ==-1){
      perror("Socket");
      exit(1);
   }

   //Port Number: Must be greater than 1000
   me_addr.sin_family = type;
   me_addr.sin_port = htons(portno);
   //INADDR_ANY sets address to pc address
   me_addr.sin_addr.s_addr = INADDR_ANY;
   //initialize mem
   bzero(&(me_addr.sin_zero),8);
   return sockfd;

}

/*
   Initializes other address to communicate to.
   Inputs:
      type = 
      portno = Port Number of destination
      IPaddr = IP address
   Output:
      1 = All Good
      -1 = Error   
*/

int clientInit(int type, int portno, char* IPaddr){
   
   other_addr.sinfamily = type;
   other_addr.sin_port = htons(portno);
   inet_pton(type, IPaddr, &(other_addr.sin_addr));
   bzero(&(other_addr.sin_zero),8);

}
/*
   Wrapper for sendto function. 
   Inputs:
      sockid = SockID
      buffer = Array with intended message
      buflen = max length of buffer
*/
void Send(int sockid, char* buffer, int buflen){
   
   sendto(sockid, buffer, buflen, 0, 
         (struct sockaddr *)&other_addr, sizeof(struct sockaddr));

} 
/*
   Wrapper for recvfrom function
   Inputs:
      sockid = SockID
      buffer = array to store received data
      buflen = max length of buffer
   Output:
      Returns number of bits received
*/
int Receive(int sockid, char* buffer, int buflen){
   
   return recvfrom(sock, buffer, buflen, 0,
                  (struct sockaddr *)&other_addr, sizeof(struct sockaddr));

}
/*
   Begins communication session between two computers   
*/
int communicationInit(char* IPaddr, int portnumber){
   int sock, quit;

   sock = socketInit(sock, AF_INET, SOCK_DGRAM, 0, portnumber);

   //Initialze Destination
   if(clientInit(AF_INET, portnumber, IPaddr) != 1){
      perror("Unable to Initalize Destination");
      exit(1);
   }
   
   //Bind Socket to localhost
   if(bind(sock,(struct sockaddr *)&me_addr, sizeof(struct sockaddr))==-1){
      perror("Unable to Bind");
      exit(1);
   }

   while(1){
      //Fork process to allow for sending and receiving at the same time.
      pid_t pID = fork();
      pid_t ppID;
   
      //Loop for reading from destination, writing to destination
      while(1){
         if(pID == 0){
            //sendto wrapper
            Send(sock, send_data, strlen(send_data));
         }
         //Throw exception if forking failed.
         else if(pID < 0){
            perror("Failed to Fork");
            ppID = getppid();
            kill(ppID, SIGTERM);
            exit(1);
         }
         else{
            //recvto wrapper
            Receive(sock, recv_data, MAX_RECV_LENGTH);
         }
   
      }
   }
}
