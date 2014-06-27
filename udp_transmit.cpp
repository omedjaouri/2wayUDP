#include "UDP_Transmission.h"
#include <stdlib.h>
#include <stdio.h>

char message[512] = {'0'};
char test[] = "hello";

int main(){
   int portno, portno_other;
   bool check;   

   printf("Destination Port Number: ");
   scanf("%d", &portno_other);
   
   printf("Your Port Number: ");
   scanf("%d", &portno);
   
   pid_t pID = fork();
   if(pID == 0){
      communicationInit("127.0.0.1", portno_other, portno, 1000000);
   }
   else{
      while(1){
         check = userWrite(test);
         if(check == false)
            printf("Send Failed\n");
         else
            printf("Send Success\n");
         printf("Transmit: \n");
         scanf("%s", message);
         printf("Sending: %s\n", message);
         
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
      }
   }
   return 0;
}  
