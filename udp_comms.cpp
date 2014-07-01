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
#include "udp_comms.h"
#include "strings.h"
#include "string.h"
#include "semaphore.h"

#define MAX_RECV_LENGTH 512
#define MAX_SEND_LENGTH 512
#define MAX_BUFFER_LENGTH 4086
#define MAX_MESSAGES 8

struct sockaddr_in server_addr, client_addr;
int inbuffer[MAX_MESSAGES] = {0};
int outbuffer[MAX_MESSAGES] = {0};
char send_buffer[MAX_SEND_LENGTH] = {0};
char recv_buffer[MAX_RECV_LENGTH] = {0};
struct _queue inqueue, outqueue;
int sock;
pthread_mutex_t outMutex, inMutex;   

//Shift an array using user provided parameters for intial position
//in array as well as the amount to shift by.
void array_shift(int* array, int shift, int init_val){
   for(int i = init_val; i<MAX_MESSAGES;i++){
      array[i-shift] = array[i];
   }
   for(int j = init_val; j<MAX_MESSAGES; j++){
      array[j] = 0;
   }
}

/*
   Initializes a socket for the Receive process to listen to for
   incoming messages.

*/
int socketInit(int sockfd, int type, int domain, int protocol,
               int portno){
   if((sockfd = socket(type, domain, protocol))==-1){
      perror("Socket\n");
      exit(1);
   }
   
   server_addr.sin_family = type;
   server_addr.sin_port = htons(portno);
   server_addr.sin_addr.s_addr = INADDR_ANY;
   //initialize memory
   bzero(&(server_addr.sin_zero),8);
   return sockfd;
}

/*
   Initializes destination address, port and communication type
*/
void clientInit(int type, int portno, char* IPaddr){
   client_addr.sin_family = type;
   client_addr.sin_port = htons(portno);
   inet_pton(type, IPaddr, &(client_addr.sin_addr));
   bzero(&(client_addr.sin_zero),8);
}

/*
   Send Process for threading

   Constantly polls the outqueue to check if there are any messages.
   If there are any messages, it uses sendto to send them to the 
   established destination address and port. Uses outbuffer to keep
   track of how large each messages is.

*/
void *Send(void* ptr){
   int k, gspace, size;
   while(1){
      //Check for values in queue
      gspace = queue_space(&outqueue, QUEUE_SPACE_GET);
      k = 0;
      if(gspace > 0){
         //Wait until userWrite has written something
         while(k<MAX_MESSAGES){
            pthread_mutex_lock(&outMutex);
            if(outbuffer[k] != 0){
               size = outbuffer[k];
               printf("Message size on buffer: %d\n", size);
               queue_get(&outqueue, send_buffer, size);
               printf("Message from buffer: %s\n", send_buffer);
               outbuffer[k] = 0;
               array_shift(outbuffer, (k+1), (k+1));
               k = MAX_MESSAGES;
               //Allow userWrite to be called again
            }
            k++;
            pthread_mutex_unlock(&outMutex);
         }  
         //Send message
         sendto(sock, send_buffer, size, 0,
               (struct sockaddr*)&client_addr,sizeof(struct sockaddr)); 
         
      }
   }
}

/*
   Receive process for threading
   
   Constantly listens on the socket for any incoming messages. When 
   the function receives a message, it passes it onto the inqueue
   and keeps track of the length of the message using inbuffer.

*/
void *Receive(void* ptr){
   int pspace, plength, bytes, j;
   while(1){
      j =0;
      bytes = recvfrom(sock, recv_buffer, MAX_RECV_LENGTH, 0,
                      (struct sockaddr*)&server_addr,(socklen_t*) 
                      sizeof(struct sockaddr));
      recv_buffer[bytes] = '\0';
      if(bytes > 0){
         pspace = queue_space(&inqueue, QUEUE_SPACE_PUT);
         plength = strlen(recv_buffer)+1;
         if(pspace >= plength){
            pthread_mutex_lock(&inMutex);
            while(j < MAX_MESSAGES){
               if(inbuffer[j] == 0){
                  inbuffer[j] = plength;
                  queue_put(&inqueue, recv_buffer, plength);
                  j = MAX_MESSAGES;  
               }
               j++;
            }
            pthread_mutex_unlock(&inMutex);
         }
         else{
            printf("Receive queue full, flushing queue\n");
            queue_flush(&inqueue);
         }
      }
   }
}

/*
   User Input Function
   
   Allows the user to pass messages onto the outqueue to be sent 
   by Send as soon as possible. This allows the user to move onto 
   other tasks without having to worry about waiting for sendto

*/
int userWrite(char* message){
   int pspace = queue_space(&outqueue, QUEUE_SPACE_PUT);
   int size = strlen(message)+1;
   int j = 0;
   if(pspace >= size){
      while(j < MAX_MESSAGES){
         pthread_mutex_lock(&outMutex);
         if(outbuffer[j] == 0){
            outbuffer[j] = size;
            queue_put(&outqueue, message, size);
            j = MAX_MESSAGES;
         }
         j++;
         pthread_mutex_unlock(&outMutex);
      }
      return size;
   }
   else{
      printf("Send Queue Full, flushing queue, please resend\n");
      queue_flush(&outqueue);
      return -1;
   }
}

/*
   User Input Function
   
   When called, checks the queue for any messages and returns the
   first message that it finds. The user must poll this function 
   until the queue is emptied
*/
int userRead(char* buffer){
   int gspace, k;
   k = 0;
   gspace = queue_space(&inqueue, QUEUE_SPACE_GET);
   
   if(gspace > 0){
      pthread_mutex_lock(&inMutex);
      while(k<MAX_MESSAGES){
         if(inbuffer[k] != 0){
            queue_get(&inqueue, buffer, inbuffer[k]);
            inbuffer[k] = 0;
            array_shift(inbuffer, (k+1), (k+1));
            k = MAX_MESSAGES;
         }
         k++;
      }
      pthread_mutex_unlock(&inMutex);
      return 1;
   }
   else{
      return -1;
   }
}

void commInit(char* IPaddr, int dest_portnumber, int portno){
   int threadret1, threadret2;
   pthread_t thread1, thread2;
   
   //Initialize Mutexs
   pthread_mutexattr_t myMutexAttr;
   pthread_mutexattr_init(&myMutexAttr);
   pthread_mutexattr_setpshared(&myMutexAttr, PTHREAD_PROCESS_SHARED);
   pthread_mutex_init(&outMutex, &myMutexAttr);
   pthread_mutex_init(&inMutex, &myMutexAttr);
   
   //Initialize queues
   if(!queue_init(&inqueue, MAX_BUFFER_LENGTH) ||
      !queue_init(&outqueue, MAX_BUFFER_LENGTH)){
      perror("Queue");
      exit(1);
   }
   //Initialize sock
   sock = socketInit(sock, AF_INET, SOCK_DGRAM, 0, portno);
   //Initialize Destination
   clientInit(AF_INET, dest_portnumber, IPaddr);
   
   //Bind socket to localhost
   if(bind(sock, (struct sockaddr*)&server_addr, 
           sizeof(struct sockaddr)) == -1){
      perror("Unable to Bind\n");
      exit(1);
   }
   
   //Create threads for Send and Receive
   threadret1 = pthread_create(&thread1, NULL, Send, NULL);
   if(threadret1){
      perror("Threading failed");
      exit(1);
   }
   threadret2 = pthread_create(&thread2, NULL, Receive, NULL);
   if(threadret2){
      perror("Threading failed");
      exit(1);
   }
}

   

