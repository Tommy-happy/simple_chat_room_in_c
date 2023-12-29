#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <omp.h>
// #include <curses.h>

#define BUFFER_SIZE 256

void error(const char *msg) { //輸出錯誤訊息
    perror(msg); //輸出msg和stderr的內容
    exit(0); //結束程式
}

int main(int argc, char *argv[]) {
    int serverfd /*socket file description*/, portno, n;
    struct sockaddr_in serv_addr; //server地址等資訊
    struct hostent *server;

    char buffer[BUFFER_SIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]); //儲存輸入的port
    serverfd = socket(AF_INET, SOCK_STREAM/*基於tcp*/, 0/*自動選擇適合的協議(tcp)*/); //設定server socket file description
    if (serverfd < 0) //socket設定失敗
        error("ERROR opening socket");
    server = gethostbyname(argv[1]); //把網址轉成IP
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr)); //設定serv_addr所指向的空間為0
    serv_addr.sin_family = AF_INET; //使用ipv4
    bcopy((char *)server->h_addr, //server的ip地址 
         (char *)&serv_addr.sin_addr.s_addr, //server->h_addr複製到的地方
         server->h_length /*server ip地址的長度*/); //二進制資料複製
    serv_addr.sin_port = htons(portno); //設定要連去的地方的port

    if (connect(serverfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //連接server並判斷有沒有連接成功
        error("ERROR connecting");
    
    omp_set_num_threads(2); //設定平行運算thread的數量
    #pragma omp parallel /*開始平行運算*/ sections //根據section平行運算，section1和section2同時進行 
    {
        #pragma omp section //section1
        {    
            while(fgets(buffer,BUFFER_SIZE-1,stdin)!=NULL){ //讀取std輸入
                n = write(serverfd,buffer,strlen(buffer)); //傳送給server
                if (n < 0)  error("ERROR writing to socket");
            }
        }
        #pragma omp section //section2
        {    
            while(1){ //讀取server輸出
                bzero(buffer,BUFFER_SIZE-1);
                n = read(serverfd,buffer,BUFFER_SIZE-1); //卡在這裡直到收到伺服端的輸入
                if (n < 0)  error("ERROR reading from socket");
                printf("%s",buffer);
            }
        }
    }

    close(serverfd);
    return 0;
}
