#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SIZE 256

typedef struct msg{
     char msgContent[BUFFER_SIZE];
     struct msg *next;
}Message;

Message* MsgRead(char *roomID, int *msgNum) {
     char fileName[9], context[BUFFER_SIZE];
     FILE *fptr;
     sprintf(fileName,"%s.rmsg",roomID);

     if((fptr = fopen(fileName, "r")) == NULL) {return NULL;}
     Message *msg = (Message*)malloc(sizeof(Message)), *head = msg;
     msg->next = NULL;
     while (fscanf(fptr,"%[^\n] ",context)!=EOF) {
          (*msgNum)++;
          msg->next = (Message*)malloc(sizeof(Message));
          snprintf(msg->next->msgContent,BUFFER_SIZE,"%s",context);
          msg->next->next = NULL;
          msg = msg->next;
     }
     fclose(fptr);
     return head;
}

void MsgWrite(char *roomID,Message *head) {
     char fileName[9], context[BUFFER_SIZE];
     FILE *fptr;
     Message* msg = head->next;
     
     sprintf(fileName,"%s.rmsg",roomID);
     if((fptr = fopen(fileName, "w")) == NULL) {return;}
     while(msg) {
          head->next = msg->next;
          fprintf(fptr,"%s\n",msg->msgContent);
          free(msg);
          msg = head->next;
     }
     free(head);
     fclose(fptr);
}

int main(int argc, char *argv[]) {
     int count;
     Message *read = MsgRead("1211",&count);
     MsgWrite("1111",read);
     printf("%i\n",count);
}
