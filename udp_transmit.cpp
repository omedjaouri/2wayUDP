#include "UDP_Transmission.h"
#include <stdlib.h>
#include <stdio.h>

char message[512];
char receive[512];

int main(){
   int portno, portno_other, check;
   
   printf("Destination Port Number: ");
   scanf("%d", &portno_other);
   
   printf("Your Port Number: ");
   scanf("%d", &portno);
   
   pid_t pID = fork();
   if(pID == 0){
      communicationInit("127.0.0.1", portno_other, portno, 10);
   }
   else{
   printf("Transmit: \n");
   scanf("%s", message);
   if(!(userWrite(message))){
      exit(1);
   }
   if(!(userRead(receive))){
      exit(1);
   }
   printf("Received: %s\n", receive);
   }
   return 0;
}
