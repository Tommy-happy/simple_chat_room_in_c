#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <omp.h>
#include <sys/select.h>

#define USERNAME_LEN 19
#define ROOM_ID_LEN 4
#define BUFFER_SIZE 256
// #define PORT 12345  //server開啟的port
typedef struct room{
     int usernum;
     int readynum;
     int max_fd;
     int* userfd;
     char room_id[ROOM_ID_LEN+1];
     char** usernames;
} ROOM;

void error(const char *msg) { //  輸出stderr的訊息在msg後面
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
     int serverfd /*socket file description*/, *clientfd=malloc(sizeof(int)), client_num=0;
     socklen_t clilen;
     fd_set readfds;
     FD_ZERO(&readfds);
     char buffer[BUFFER_SIZE], username[USERNAME_LEN+1], room_id[ROOM_ID_LEN+1];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     struct timeval timeout = {1, 0};  // 1秒，0微秒
     ROOM* chat_rooms;
     chat_rooms = (ROOM*) malloc(sizeof(ROOM));
     chat_rooms[0].usernum = 0;
     chat_rooms[0].readynum = 0;
     chat_rooms[0].usernames = (char**) malloc(0);
     chat_rooms[0].userfd = (int*) malloc(0);
     chat_rooms[0].max_fd = 0;
     strcpy(chat_rooms[0].room_id,"0000");

     if (argc < 2) {
          fprintf(stderr,"usage %s port\n", argv[0]);
          exit(0);
     }
     int PORT = atoi(argv[1]);

     serverfd = socket(AF_INET/*使用ipv4*/, SOCK_STREAM/*基於tcp*/, 0/*自動選擇適合的協議(tcp)*/);
     if (serverfd < 0)
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr)); //設定serv_addr所指向的空間為0
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY; //接收所有的IP請求
     serv_addr.sin_port = htons(PORT); //設定port hton(host to network)

     if (bind(serverfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))/*綁定serverfd和server的地址*/ < 0) 
              error("ERROR on binding");

     listen(serverfd,5/*backlog:排隊等待連線的最大數量*/); //監聽serverfd
     clilen = sizeof(cli_addr);
     // omp_set_num_threads(3); //設定平行運算thread的數量
     #pragma omp parallel /*開始平行運算*/ sections //根據section平行運算，section1和section2同時進行 
     {
     #pragma omp section //處理新增的client
     {
          while(1){
               int userfd = accept(serverfd, (struct sockaddr *) &cli_addr/*儲存回傳的客戶端地址*/, &clilen); //卡在這行直到接收到客戶端連線
               chat_rooms[0].userfd = (int*) realloc(chat_rooms[0].userfd, sizeof(int)*++chat_rooms[0].usernum);
               chat_rooms[0].userfd[chat_rooms[0].usernum-1] = userfd;
               if (chat_rooms[0].userfd[chat_rooms[0].usernum-1] < 0) 
                    error("ERROR on accept");
               // chat_rooms[0].usernum++;
               bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間的值設為0
               n = read(chat_rooms[0].userfd[chat_rooms[0].usernum-1],buffer,BUFFER_SIZE-1); //客戶端的輸入username
               if (n < 0) error("ERROR reading from socket");
               // buffer[strlen(buffer)-1] = '\0';
               chat_rooms[0].usernames = (char**) realloc(chat_rooms[0].usernames, chat_rooms[0].usernum*sizeof(char*));
               chat_rooms[0].usernames[chat_rooms[0].usernum-1] = malloc(sizeof(char)*(strlen(buffer)+1));
               strcpy(chat_rooms[0].usernames[chat_rooms[0].usernum-1], buffer);               
               bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間的值設為0
               n = read(chat_rooms[0].userfd[chat_rooms[0].usernum-1],buffer,BUFFER_SIZE-1); //客戶端的輸入room id
               if (n < 0) error("ERROR reading from socket");

               // FD_SET(chat_rooms[0].userfd[chat_rooms[0].usernum-1], &readfds); //加到fd的set中
               // printf("usernum=%d\nuserfd=%d\n", chat_rooms[0].usernum, chat_rooms[0].userfd[chat_rooms[0].usernum-1]);
               chat_rooms[0].max_fd = chat_rooms[0].max_fd<=chat_rooms[0].userfd[chat_rooms[0].usernum-1] ? chat_rooms[0].userfd[chat_rooms[0].usernum-1]+1 : chat_rooms[0].max_fd; 
               chat_rooms[0].readynum++;
               printf("welcome %s at %s\n",chat_rooms[0].usernames[chat_rooms[0].usernum-1], buffer);
               }
     }
     #pragma omp section //監聽client有沒有要傳送訊息，並將訊息廣播到群內的所有人
     {
          while(1){ //循環select
               // int ret =select(chat_rooms[0].max_fd, &readfds, NULL, NULL, NULL);
               // printf("FD_SETSIZE=%d\n", FD_SETSIZE);
               if(chat_rooms[0].readynum != 0){
                    FD_ZERO(&readfds);
                    for(int k = 0; k < chat_rooms[0].readynum; k++) {
                         FD_SET(chat_rooms[0].userfd[k], &readfds); //加到fd的set中
                    }
               }
               else{
                    sleep(0.01);
                    continue;
               }
               int ret =select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
               timeout.tv_sec=1;
               timeout.tv_usec=0;
               int first = 1;
               if(ret < 0){
                    error("select error\n");
               }
               else if(ret==0){
                    // printf("ret=%d\n", ret);
                    continue;
               }
               else{
                    printf("revieved notification!!\n");
                    for(int i=0;i<chat_rooms[0].readynum;i++){ //檢查該client有沒有要輸入
                         if(FD_ISSET(chat_rooms[0].userfd[i], &readfds)){
                              bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間設為0
                              n = read(chat_rooms[0].userfd[i],buffer,BUFFER_SIZE-1); //卡在這裡直到收到客戶端的輸入
                              if (n < 0) error("ERROR reading from socket");
                              for(int j=0;j<chat_rooms[0].readynum;j++){
                                   if(i==j){
                                        continue;
                                   }
                                   write(chat_rooms[0].userfd[j],chat_rooms[0].usernames[i],strlen(chat_rooms[0].usernames[i])); //輸出到client端
                                   if (n < 0) error("ERROR writing to socket");
                                   write(chat_rooms[0].userfd[j],": ",strlen(": ")); //輸出到client端
                                   if (n < 0) error("ERROR writing to socket");
                                   n = write(chat_rooms[0].userfd[j],buffer,strlen(buffer)); //傳送訊息到客戶端
                                   if (n < 0) error("ERROR writing to socket");
                              }
                         }
                    }
               }
          }
     }
     
     }

     // close(clientfd1);
     // close(clientfd2);
     close(serverfd);
     return 0; 
}
