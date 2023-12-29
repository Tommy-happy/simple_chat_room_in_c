#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <omp.h>

#define BUFFER_SIZE 256
#define PORT 12345  //server開啟的port

void error(const char *msg) { //  輸出stderr的訊息在msg後面
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
     int serverfd /*socket file description*/, clientfd1, clientfd2;
     socklen_t clilen;
     char buffer[BUFFER_SIZE];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     serverfd = socket(AF_INET/*使用ipv4*/, SOCK_STREAM/*基於tcp*/, 0/*自動選擇適合的協議(tcp)*/);
     if (serverfd < 0)
        error("ERROR opening socket");
     int reuse = 1;

     bzero((char *) &serv_addr, sizeof(serv_addr)); //設定serv_addr所指向的空間為0
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY; //接收所有的IP請求
     serv_addr.sin_port = htons(PORT); //設定port hton(host to network)

     if (bind(serverfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))/*綁定serverfd和server的地址*/ < 0) 
              error("ERROR on binding");

     listen(serverfd,5/*backlog:排隊等待連線的最大數量*/); //監聽serverfd
     clilen = sizeof(cli_addr);
     clientfd1 = accept(serverfd, (struct sockaddr *) &cli_addr/*儲存回傳的客戶端地址*/, &clilen); //卡在這行直到接收到客戶端連線
     if (clientfd1 < 0) 
          error("ERROR on accept");
     printf("got one\n");
     clientfd2 = accept(serverfd, (struct sockaddr *) &cli_addr/*儲存回傳的客戶端地址*/, &clilen); //卡在這行直到接收到客戶端連線
     if (clientfd2 < 0) 
          error("ERROR on accept");
     printf("got two\n");



     omp_set_num_threads(2); //設定平行運算thread的數量
     #pragma omp parallel /*開始平行運算*/ sections //根據section平行運算，section1和section2同時進行 
     {
     #pragma omp section //section1
     {
          while(1){
               int first = 1;
               // printf("renew\n");
               while(1){
                    bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間設為0
                    n = read(clientfd1,buffer,BUFFER_SIZE-1); //卡在這裡直到收到客戶端的輸入
                    if (n < 0) error("ERROR reading from socket");
                    
                    if(first){
                         write(clientfd2,"client1: ",strlen("client1: "));
                         first = 0;
                    }

                    n = write(clientfd2,buffer,strlen(buffer)); //傳送訊息到客戶端
                    if (n < 0) error("ERROR writing to socket");

                    if(buffer[strlen(buffer)-1]=='\n'){
                         printf("the end\n");
                         break;
                    }
               }
          }
     }
     #pragma omp section //section2
     {
          while(1){
               int first = 1;
               while(1){
                    bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間設為0
                    n = read(clientfd2,buffer,BUFFER_SIZE-1); //卡在這裡直到收到客戶端的輸入
                    if (n < 0) error("ERROR reading from socket");
                    
                    if(first){
                         write(clientfd1,"client2: ",strlen("client2: ")); //輸出到client端
                         first = 0;
                    }

                    n = write(clientfd1,buffer,strlen(buffer)); //傳送訊息到客戶端
                    if (n < 0) error("ERROR writing to socket");

                    if(buffer[strlen(buffer)-1]=='\n'){
                         printf("the end\n");
                         break;
                    }
               }
          }
     }
     
     }

     close(clientfd1);
     close(clientfd2);  
     close(serverfd);
     return 0; 
}
