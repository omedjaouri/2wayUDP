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
int pipein[2], pipeout[2];
int sock;

/*
   Initializes a socket with given id and returns the socketID
   as an integer. 
   Inputs:
      type t
exi 
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
*/

void clientInit(int type, int portno, char* IPaddr){
   
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
   Takes User message and passes it onto the queue so that it is sent as   soon as possible.
   
   Inputs:
      message = Null terminated string
*/

void userWrite(char* message){
   //WRITE TO INFIFO FROM USER
}

/*
   Takes data from queue and returns it to User
*/

char* userRead(void){
   char* buffer[MAX_RECV_LENGTH];

   //READ FROM OUTFIFO AND PASS TO USER  
 
   return buffer;
}
/*
   Begins communication session between two computers   
*/
void communicationInit(char* IPaddr, int portnumber){
   int recv_length;

   //Initialize sock
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
      
   //Create Pipe Buffers for Send and Receive
   if(pipe(pipein) == -1 || pipe(pipeout) == -1){
      perror("Pipe");
      exit(1);
   }

   while(1){
      //Fork process to allow for sending and receiving at the same time.
      pid_t pID = fork();
      pid_t ppID;
   
      //Loop for reading from destination, writing to destination
      while(1){
         if(pID == 0){
            //Child uses Output Pipe to send to destination
            
            //READ FROM INFIFO FROM USER
            
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
            recv_length = Receive(sock, recv_data, MAX_RECV_LENGTH);
            recv_data[recv_length] = '\0';
            //Parent uses Input Pipe to send received data to user
               
            //WRITE TO OUTFIFO FOR USER
         }
   
      }
   }
}
