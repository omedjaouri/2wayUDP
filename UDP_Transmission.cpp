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
#include "_fifo_queue.h"

#define MAX_RECV_LENGTH  512
#define MAX_SEND_LENGTH  512
//Buffer length must be multiple of recv/send buffer
#define MAX_BUFFER_LENGTH 4086
#define MAX_MESSAGES 8

//Two-Way communication only
struct sockaddr_in me_addr, other_addr;
char recv_data[MAX_RECV_LENGTH], send_data[MAX_SEND_LENGTH];
int inbuffer[MAX_MESSAGES], outbuffer[MAX_MESSAGES];
int sock;
struct _queue inqueue, outqueue;


/*
   
*/
void array_shift(int* array, int shift, int init_val){
   for(int i = init_val; i < MAX_MESSAGES; i++){
      array[i-shift]=array[i];
   }
   for(int j = init_val; j<MAX_MESSAGES; j++){
      array[j] = 0;
   }
   return;
}
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

int clientInit(int type, int portno, char* IPaddr){
   
   other_addr.sin_family = type;
   other_addr.sin_port = htons(portno);
   inet_pton(type, IPaddr, &(other_addr.sin_addr));
   bzero(&(other_addr.sin_zero),8);
   return 1;
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
                  (struct sockaddr *)&other_addr,(socklen_t*) sizeof(struct sockaddr));

}

/*
   Takes User message and passes it onto the queue so that it is sent as   soon as possible.
   
   Inputs:
      message = Null terminated string
   Output:
      Returns true if successfully added user string to queue
      Returns false if queue is full
*/

bool userWrite(char* message){
   int k;
   k = 0;
   //WRITE TO INFIFO FROM USER
   while(k<MAX_MESSAGES){
      
      if(inbuffer[k] == 0){
         inbuffer[k] = strlen(message);
         queue_put(&inqueue, message, inbuffer[k]);
         return true;
      }
      k++;
   }
   return false;
}

/*
   Takes data from queue and returns it to User
   
   Outputs:
      Returns buffer containing latest message. 
      Returns NULL if receive queue is empty
*/

bool userRead(char* buffer){
   int k, j;
   k = 0;
   //READ FROM OUTFIFO AND PASS TO USER  
   while(k<MAX_MESSAGES){
      //If nonzero message length in the buffer, read message and shift
      if(outbuffer[k] != 0){
         j = outbuffer[k];
         queue_get(&outqueue, buffer, outbuffer[k]);  
         array_shift(outbuffer, (k+1), (k+1));
         buffer[j] = '\0';
         return true;
      }
      k++;
   }
   return false;
}
/*
   Begins communication session between two computers   
*/
void communicationInit(char* IPaddr, int dest_portnumber, 
                       int portno, int time){
   
   int recv_length, k, j, ticks;
   ticks = 0;

   //Initialize queues
   if(!queue_init(&inqueue, MAX_BUFFER_LENGTH) || 
      !queue_init(&outqueue, MAX_BUFFER_LENGTH)){
      perror("Queue");
      exit(1);
   }
   

   //Initialize sock
   sock = socketInit(sock, AF_INET, SOCK_DGRAM, 0, portno);

   //Initialze Destination
   if(clientInit(AF_INET, dest_portnumber, IPaddr) != 1){
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
            //Child uses inqueue to send to destination
            int k = 0;
            while(k<MAX_MESSAGES){
            //If nonzero message length in the buffer, 
            //read message and shift
               if(inbuffer[k] != 0){
                  queue_get(&inqueue, send_data, inbuffer[k]);
                  if(send_data[0] == 'q'){
                     ppID= getppid();
                     kill(ppID, SIGTERM);
                     exit(1);
                  }
                  inbuffer[k] = 0;
                  array_shift(inbuffer, (k+1), (k+1));
                  k = MAX_MESSAGES;
                  }
               k++;
            }
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
            j = 0;
            ticks = ticks + 1;
            printf("Ticks: %d\n", ticks);
            if(ticks == time){
               kill(pID, SIGTERM);
               exit(1);
            }
            //recvto wrapper
            recv_length = Receive(sock, recv_data, MAX_RECV_LENGTH);
            recv_data[recv_length] = '\0';
            //Parent uses outqueue to send received data to user
           
             while(j<MAX_MESSAGES){
               
               if(outbuffer[j] == 0){
                  outbuffer[j] = strlen(recv_data);
                  queue_put(&outqueue, recv_data, outbuffer[j]);
                  j = MAX_MESSAGES;
               }
               j++;
            } 
         }
   
      }
   }
}
