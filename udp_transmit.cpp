#include "udp_comms.h"
#include <stdlib.h>
#include <stdio.h>

char message[512] = {'0'};
char test[] = "i am very confused";

int main(){
   int portno, portno_other, k;
   bool check;   

   printf("Destination Port Number: ");
   scanf("%d", &portno_other);
   
   printf("Your Port Number: ");
   scanf("%d", &portno);
   
   commInit("127.0.0.1", portno_other, portno);
   
   k = 0;
   while(k<10){
      check = userWrite(test);
      if(check == -1)
         printf("Unable to put on queue\n"); 
      //printf("Transmit: \n");
      //scanf("%s", message);
      //printf("Sending: %s\n", message);
      /*   
      if(message == "quit"){
         printf("Program ending\n");
         kill(pID, SIGTERM);
         waitpid(pID, NULL, 0);
         exit(1);   
      }
      else{
         check = userWrite(test);
         if(check == false){
            printf("Send failed\n");
         }
         else{
            printf("Send Success\n");
         }
      }
   }*/
      k++;
   }
   while(1){
   }
   return 0;
}  
