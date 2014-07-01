#include "udp_comms.h"
#include <stdlib.h>
#include <stdio.h>

char receive[1024];

int main(){
   int portno, portno_other, bytes;
   
   printf("Destination Port Number: ");
   scanf("%d", &portno_other);
   
   printf("Your Port Number: ");
   scanf("%d", &portno);
   
   commInit("127.0.0.1", portno_other, portno);
   
   while(1){
      bytes = userRead(receive);
      //printf("%d\n", bytes);
      if(bytes > 0){
         printf("Received %s\n", receive);
      }
   }
}

