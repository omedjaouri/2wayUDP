#include "UDP_Transmission.h"
#include <stdlib.h>
#include <stdio.h>

char receive[1024];

int main(){
   int portno, portno_other, bytes;
   
   printf("Destination Port Number: ");
   scanf("%d", &portno_other);
   
   printf("Your Port Number: ");
   scanf("%d", &portno);
   
   pid_t pID = fork();
   if(pID == 0){
      communicationInit("127.0.0.1", portno_other, portno, 1000);
   }
   else{
      while(1){
         bytes = userRead(receive);
         if(bytes > 0){
            printf("Received %s\n", receive);
         }
      }
   }
}
