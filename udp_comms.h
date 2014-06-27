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
#include <iostream.h>
#include <pthread.h>
#include <_fifo_queue.h>

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

void array_shift(int* array, int shift, int init_val);

int socketInit(int sockfd, int type, int domain, int protocol,
               int portno);

void clientInit(int type, int portno, char* IPaddr);

void (*Send)(void);

void (*Receive)(void);

void userWrite(char* message);

int userRead(char* buffer);

void commInit(char* IPaddr, int dest_portnumber, int portno);
