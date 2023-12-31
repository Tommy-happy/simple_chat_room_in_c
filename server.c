#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <omp.h>
#include <sys/select.h>
#include <time.h>

#define USERNAME_LEN 19
#define ROOM_ID_LEN 4
#define BUFFER_SIZE 256
#define LOAD_FACTOR 0.7
#define INIT_ROOM_TABLE_SIZE 10
// #define PORT 12345  //server開啟的port


typedef struct msg{
     char* msgContent;
     time_t msgTime;
     struct msg *pre;
}Message;

typedef struct room{
     int usernum; //使用者數量
     int readynum; //設定完基本資料的使用者數量
     int max_fd; //socket的最大值 (socket數值沒有意義，只是不會重複而已)
     int* userfd; //clients的socket陣列
     // char room_id[ROOM_ID_LEN+1]; 
     char** usernames; //clients的名字陣列
     Message* msg; //指向最新傳的訊息
} ROOM;

typedef struct room_node
{
    char* id;
    ROOM* room;
    struct room_node * next;
} ROOM_NODE;

typedef struct room_table
{
    ROOM_NODE** table;
    int table_size;
    int num_node;
} ROOM_TABLE;

int hash(char*, int); // hash function
void free_room_table(ROOM_NODE ** table); // 刪除整個room table
void init_room_node(ROOM_NODE* room_node, char* id, ROOM* room); // 初始化一個room !!不用自己call
void resize_room_table(ROOM_TABLE* room_table); // 調整room table大小(根據load_factor)
ROOM_TABLE* delete_room(ROOM_TABLE* room_table, char* id); // delete room from room table
ROOM* get_room(ROOM_TABLE* room_table, char* id); // 取得給定id的room的地址
int create_room(ROOM_TABLE** room_table_addr, char* id, ROOM* room); //在room table中新增一個room
int check_room_exist(ROOM_TABLE* room_table_addr, char* id); // 檢查room有沒有存在於 room table 中
void change_room(char* room_id, ROOM* origin_room, int idx);

void error(const char *msg) { //  輸出stderr的訊息在msg後面
    perror(msg);
    exit(1);
}

ROOM_TABLE* room_table;
int main(int argc, char *argv[]) {
     int serverfd /*socket file description*/, *clientfd=malloc(sizeof(int)), client_num=0;
     socklen_t clilen;
     // fd_set readfds;
     // FD_ZERO(&readfds);
     char buffer[BUFFER_SIZE], username[USERNAME_LEN+1], room_id[ROOM_ID_LEN+1];
     char is_join_str[2];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     int* all_clientfd = (int*) malloc(sizeof(int));
     int total_client=0;
     struct timeval timeout = {0, 100000};  // 秒，100000微秒
     //
     // ROOM* chat_rooms;
     // chat_rooms = (ROOM*) malloc(sizeof(ROOM));
     // chat_rooms[0].usernum = 0;
     // chat_rooms[0].readynum = 0;
     // chat_rooms[0].usernames = (char**) malloc(0);
     // chat_rooms[0].userfd = (int*) malloc(0);
     // chat_rooms[0].max_fd = 0;
     // strcpy(chat_rooms[0].room_id,"0000");
     //
     room_table = (ROOM_TABLE*) malloc(sizeof(ROOM_TABLE));
     room_table->table_size = INIT_ROOM_TABLE_SIZE;
     room_table->num_node = 0;
     room_table->table = (ROOM_NODE**) calloc(INIT_ROOM_TABLE_SIZE, sizeof(ROOM_NODE*));
     // for(int i=0;i<INIT_ROOM_TABLE_SIZE;i++){
     //      room_table->table[i] = NULL;
     // }
     // HASH_NODE* hash_node = (HASH_NODE*) malloc(sizeof(HASH_NODE));
     // init_room_node(hash_node, id, room);
     // room_table->table[hash(field, INIT_ROOM_TABLE_SIZE)] = hash_node;

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
     // omp_set_num_threads(2); //設定平行運算thread的數量
     #pragma omp parallel/*開始平行運算*/ sections //根據section平行運算，section1和section2同時進行 
     {
     #pragma omp section //處理新增的client
     {
          #pragma omp parallel private(username, n, buffer, is_join_str)//處理新增的client
          {
               while(1){
                    char* username = NULL;
                    int userfd = accept(serverfd, (struct sockaddr *) &cli_addr/*儲存回傳的客戶端地址*/, &clilen); //卡在這行直到接收到客戶端連線
                    if (userfd < 0) 
                         error("ERROR on accept");
                    //讀取username
                    bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間的值設為0
                    n = read(userfd,buffer,BUFFER_SIZE-1); //客戶端的輸入username
                    if (n < 0) error("ERROR reading from socket");
                    username = (char*) malloc(sizeof(char)*(strlen(buffer)+1));
                    strcpy(username, buffer);

                    read_room_id:{}
                    //讀取是否為加入房間
                    bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間的值設為0
                    bzero(is_join_str,1); //將buffer中BUFFER_SIZE大小的空間的值設為0
                    n = read(userfd,is_join_str,1); //客戶端的輸入是否要join
                    if (n < 0) error("ERROR reading from socket");
                    //讀取房間ID
                    bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間的值設為0
                    n = read(userfd,buffer,BUFFER_SIZE-1); //客戶端的輸入room id
                    if (n < 0) error("ERROR reading from socket");
                    printf("room id=%s\n", buffer);

                    ROOM* room = NULL;
                    if(is_join_str[0]=='1'){ //要加入房間
                         printf("some body want to join\n");
                         if(check_room_exist(room_table, buffer)){ // 可加入房間
                              printf("he/she joined!\n");
                              room = get_room(room_table, buffer);
                              room->userfd = (int*) realloc(room->userfd, sizeof(int)*++room->usernum); //插入client socket
                              room->userfd[room->usernum-1] = userfd;
                              room->usernames = (char**) realloc(room->usernames, room->usernum*sizeof(char*)); //插入client name
                              room->usernames[room->usernum-1] = malloc(sizeof(char)*(strlen(username)+1));
                              strcpy(room->usernames[room->usernum-1], username);
                              room->max_fd = room->max_fd<=room->userfd[room->usernum-1] ? room->userfd[room->usernum-1]+1 : room->max_fd; // 設定max fd值
                              room->readynum++;
                              n = write(userfd, "200OK", 5);
                              if(n < 0) error("ERROR writing to socket");
                         }
                         else{ // 要加入的房間不存在
                              printf("room not found\n");
                              n = write(userfd, "room not found", 14);
                              if(n < 0) error("ERROR writing to socket");
                              goto read_room_id;
                         }
                    }
                    else{ //創新房間
                         if(check_room_exist(room_table, buffer)){ // 房間已存在
                              printf("room is already existed\n");
                              n = write(userfd, "room is already existed", 14);
                              if(n < 0) error("ERROR writing to socket");
                              goto read_room_id;
                         }
                         else{ //可創建房間
                              printf("creating room\n");
                              ROOM* room = (ROOM*) malloc(sizeof(ROOM));
                              room->usernum = 1;
                              room->readynum = 1;
                              room->usernames = (char**) malloc(sizeof(char*));
                              room->usernames[0] = (char*) malloc(sizeof(char)*(strlen(username)+1));
                              strcpy(*(room->usernames), username);
                              room->userfd = (int*) malloc(sizeof(int));
                              room->userfd[0] = userfd;
                              room->max_fd = userfd+1;
                              // strcpy(room->room_id, buffer);
                              // room->msg = (Message*) malloc(sizeof(Message));
                              // room->msg->pre = NULL;
                              room->msg=NULL;
                              if(create_room(&room_table, buffer, room)){
                                   printf("room created\n");
                                   n = write(userfd, "200OK", 5);
                                   if(n < 0) error("ERROR writing to socket");
                              }
                              else{
                                   printf("some error happened while creating room\n");
                                   n = write(userfd, "create error", 12);
                                   if(n < 0) error("ERROR writing to socket");
                              }
                         }
                    }
                    // FD_SET(chat_rooms[0].userfd[chat_rooms[0].usernum-1], &readfds); //加到fd的set中
                    // printf("usernum=%d\nuserfd=%d\n", chat_rooms[0].usernum, chat_rooms[0].userfd[chat_rooms[0].usernum-1]);
                    all_clientfd = realloc(all_clientfd, sizeof(int)*++total_client);
                    all_clientfd[total_client-1] = userfd;
                    
                    printf("welcome %s at %s\n",username, buffer);
                    }
          }
     }

     #pragma omp section //處理client傳入訊息
     {
          fd_set readfds;
          iter_room:
          // #pragma omp single 
          // {
          FD_ZERO(&readfds);
          // }
          #pragma omp parallel for//把所有client的fd加到fd set中
          for(int i=0;i<total_client;i++){
               FD_SET(all_clientfd[i], &readfds);
          }
          // #pragma omp single 
          // {
          int ret =select(FD_SETSIZE, &readfds, NULL, NULL, &timeout); //watch out
          if(ret < 0){
               error("select error\n");
               exit(0);
          }
          else if(ret == 0){
               timeout.tv_sec=0;
               timeout.tv_usec=100000;
          }
          else{
               printf("recieved notification!!\n");
               timeout.tv_sec=0;
               timeout.tv_usec=100000;
               goto find_writer;
          }
          // }
          goto iter_room;

          find_writer:
          #pragma omp parallel for private(n, buffer) //監聽client有沒有要傳送訊息，並將訊息廣播到群內的所有人
          for(int room_idx=0 ; room_idx < room_table->table_size/*初始值10*/ ; room_idx++){ //遍歷room
               ROOM_NODE* tmp_room_node = room_table->table[room_idx];
               while(tmp_room_node){
                    for(int i=0;i<tmp_room_node->room->readynum;i++){ //檢查該client有沒有要輸入
                         if(FD_ISSET(tmp_room_node->room->userfd[i], &readfds)){
                              bzero(buffer,BUFFER_SIZE); //將buffer中BUFFER_SIZE大小的空間設為0
                              n = read(tmp_room_node->room->userfd[i],buffer,BUFFER_SIZE-1); //卡在這裡直到收到客戶端的輸入
                              if (n < 0) error("ERROR reading from socket");
                              else if(n==0) { //使用者離線
                                   for(int i=0;i<total_client;i++){
                                        if(all_clientfd[i] == tmp_room_node->room->userfd[i]){
                                             all_clientfd[i] = all_clientfd[total_client-1];
                                             realloc(all_clientfd, --total_client*sizeof(int));
                                             break;
                                        }
                                   }
                                   strcpy(tmp_room_node->room->usernames[i], tmp_room_node->room->usernames[tmp_room_node->room->usernum-1]);
                                   free(tmp_room_node->room->usernames[tmp_room_node->room->usernum-1]);
                                   tmp_room_node->room->usernames = (char**) realloc(tmp_room_node->room->usernames, sizeof(char*)*(tmp_room_node->room->usernum-1));
                                   tmp_room_node->room->userfd[i] = tmp_room_node->room->userfd[tmp_room_node->room->usernum-1];
                                   tmp_room_node->room->userfd = (int*) realloc(tmp_room_node->room->userfd, sizeof(char*)*(tmp_room_node->room->usernum-1));
                                   tmp_room_node->room->readynum--;
                                   tmp_room_node->room->usernum--;
                              }
                              else{
                                   printf("%s say %s\n", tmp_room_node->room->usernames[i], buffer);
                                   if(!strncmp(buffer, "$$CR", 4)){ //change room
                                        char room_id[] = "room";
                                        printf("here %s\n", buffer);
                                        strncpy(room_id, buffer+5, 4);
                                        // for(int c=0;c<4;c++){
                                        //      room_id[c] = buffer[5+c];
                                        // }
                                        // room_id[4] = '\0';
                                        printf("here2\n");
                                        change_room(room_id, tmp_room_node->room, i);
                                        printf("here3\n");
                                   }
                                   else{
                                        char* name_msg = (char*) malloc((2+strlen(tmp_room_node->room->usernames[i])+strlen(buffer)));
                                        strcpy(name_msg, tmp_room_node->room->usernames[i]);
                                        strcat(name_msg, ":");
                                        strcat(name_msg, buffer);
                                        Message* msg = (Message*) malloc(sizeof(Message));
                                        msg->msgContent = name_msg;
                                        time(&(msg->msgTime));
                                        msg->pre = tmp_room_node->room->msg;
                                        tmp_room_node->room->msg = msg;
                                        for(int j=0;j<tmp_room_node->room->readynum;j++){
                                             if(i==j){
                                                  continue;
                                             }
                                             write(tmp_room_node->room->userfd[j],tmp_room_node->room->usernames[i],strlen(tmp_room_node->room->usernames[i])); //輸出到client端
                                             if (n < 0) error("ERROR writing to socket");
                                             write(tmp_room_node->room->userfd[j],": ",strlen(":")); //輸出到client端
                                             if (n < 0) error("ERROR writing to socket");
                                             n = write(tmp_room_node->room->userfd[j],buffer,strlen(buffer)); //傳送訊息到客戶端
                                             if (n < 0) error("ERROR writing to socket");
                                        }
                                   }
                              }

                         }
                    }
                    tmp_room_node = tmp_room_node->next;
               }
          }
          goto iter_room;
     }
     }

     // close(clientfd1);
     // close(clientfd2);
     close(serverfd);
     return 0; 
}

void change_room(char* room_id, ROOM* origin_room, int idx){
     ROOM* new_room;
     if(check_room_exist(room_table, room_id)){ // 可加入房間
          new_room = get_room(room_table, room_id);
          new_room->userfd = (int*) realloc(new_room->userfd, sizeof(int)*++new_room->usernum); //插入client socket
          new_room->userfd[new_room->usernum-1] = origin_room->userfd[idx];
          new_room->usernames = (char**) realloc(new_room->usernames, new_room->usernum*sizeof(char*)); //插入client name
          new_room->usernames[new_room->usernum-1] = malloc(sizeof(char)*(strlen(origin_room->usernames[idx])+1));
          strcpy(new_room->usernames[new_room->usernum-1], origin_room->usernames[idx]);
          new_room->max_fd = new_room->max_fd<=new_room->userfd[new_room->usernum-1] ? new_room->userfd[new_room->usernum-1]+1 : new_room->max_fd; // 設定max fd值
          new_room->readynum++;
          // n = write(userfd, "200OK", 5);
          // if(n < 0) error("ERROR writing to socket");
          int n = write(origin_room->userfd[idx], "200OK\n", 6); //傳送訊息到客戶端
          if (n < 0) error("ERROR writing to socket");

          strcpy(origin_room->usernames[idx], origin_room->usernames[origin_room->usernum-1]);
          free(origin_room->usernames[origin_room->usernum-1]);
          origin_room->usernames = (char**) realloc(origin_room->usernames, sizeof(char*)*(origin_room->usernum-1));
          origin_room->userfd[idx] = origin_room->userfd[origin_room->usernum-1];
          origin_room->userfd = (int*) realloc(origin_room->userfd, sizeof(char*)*(origin_room->usernum-1));
          origin_room->readynum--;
          origin_room->usernum--;
     }
     else{
          printf("%s not found\n", room_id);
          char err_msg[38] = "[error] room id \"";
          strcat(err_msg, room_id);
          strcat(err_msg, "\" doesn't exist.\n");
          printf("%s\n", err_msg);
          int n = write(origin_room->userfd[idx], err_msg, strlen(err_msg)); //傳送訊息到客戶端
          if (n < 0) error("ERROR writing to socket");
          // sleep(1);
          // printf("here?\n");
     }
}

int hash(char* key, int ROOM_TABLE_size){
    int c, sum=0;
    while(c=*key++){
        sum+=c;
    }
    return sum % ROOM_TABLE_size;     
}

void free_room_table(ROOM_NODE ** table){
     ROOM_NODE* room_node;
     while(room_node = *table++){
          ROOM_NODE* pre_node;
          while(room_node){
               pre_node = room_node;
               room_node = room_node->next;
               free(pre_node->id);
               free(pre_node->room);
               free(pre_node);
          }
     }
     // printf("table NULL? %d\n", table==NULL);
     // free(table);
}

void init_room_node(ROOM_NODE* room_node, char* id, ROOM* room){
     // ROOM_NODE = (ROOM_NODE*) malloc(sizeof(ROOM_NODE));
     room_node->id = (char*) malloc(sizeof(char)*(strlen(id)+1));
     room_node->room = (ROOM*) malloc(sizeof(ROOM));
     strcpy(room_node->id, id);
     room_node->room = room;
     room_node->next = NULL;
}

void resize_room_table(ROOM_TABLE* room_table){
    int new_size = room_table->table_size * 2;
    ROOM_NODE** new_table = (ROOM_NODE**) calloc(new_size, sizeof(ROOM_NODE*));
    for(int i=0;i<room_table->table_size;i++){ //把舊table的資料搬運到新table
        ROOM_NODE* tmp_room_node = room_table->table[i];
        while(tmp_room_node){
            ROOM_NODE* appended_room_node;
            while(appended_room_node){
                appended_room_node = appended_room_node->next;
            }
            appended_room_node = (ROOM_NODE*) malloc(sizeof(ROOM_NODE));
            init_room_node(appended_room_node, tmp_room_node->id, tmp_room_node->room);
            new_table[hash(tmp_room_node->id,new_size)] = appended_room_node;
            tmp_room_node = tmp_room_node->next;
        }
    }
    free_room_table(room_table->table);
    free(room_table->table);
    room_table->table = new_table;
    room_table->table_size = new_size;
}

ROOM_TABLE* delete_room(ROOM_TABLE* room_table, char* id){ //delete room
    ROOM_NODE* room_node = room_table->table[hash(id, room_table->table_size)];
    ROOM_NODE* pre_room_node = NULL;
    while(room_node){
        if(!strcmp(room_node->id, id)){
            if(pre_room_node){
                pre_room_node->next = room_node->next;
            }
            else{
                room_table->table[hash(id, room_table->table_size)] = room_node->next;
            }
            free(room_node->id);
            free(room_node->room); //-----------------------
            free(room_node);
            return room_table;
        }
        pre_room_node = room_node;
        room_node = room_node->next;
    }
    return room_table; // [error] the room doesn't exist
}

ROOM* get_room(ROOM_TABLE* room_table, char* id){
    ROOM_NODE* room_node = room_table->table[hash(id, room_table->table_size)];
    while(room_node){
        if(!strcmp(room_node->id, id)){
            return room_node->room;
        }
        room_node = room_node->next;
    }
    return NULL; // [error] the room doesn't exist
}

int create_room(ROOM_TABLE** room_table_addr, char* id, ROOM* room){
     ROOM_TABLE* room_table = *room_table_addr;
     int key_idx = hash(id, room_table->table_size);
     ROOM_NODE* tmp_room_node = room_table->table[key_idx];
     ROOM_NODE* pre_room_node = NULL;
     while(tmp_room_node){
          if(!strcmp(tmp_room_node->id, id)){ //room id已存在
               return 0; //room id has already existed!!
          }
          pre_room_node = tmp_room_node;
          tmp_room_node->next;
     }
     if(!pre_room_node){ //hash table該index為空
          room_table->table[key_idx] = (ROOM_NODE*) malloc(sizeof(ROOM_NODE));
          init_room_node(room_table->table[key_idx], id, room);
          room_table->num_node++;
          // 判斷load factor
          if((float)room_table->num_node/room_table->table_size > LOAD_FACTOR){
               printf("%f\n", (float)room_table->num_node/room_table->table_size);
               resize_room_table(room_table);
          }
          return 1; //創新id
     }
     else{
          pre_room_node->next = (ROOM_NODE*) malloc(sizeof(ROOM_NODE));
          init_room_node(pre_room_node->next, id, room);
          return 1; //創新id
     }

    return 1;
}

int check_room_exist(ROOM_TABLE* room_table, char* id){
     int key_idx = hash(id, room_table->table_size);
     ROOM_NODE* tmp_room_node = room_table->table[key_idx];
     ROOM_NODE* pre_room_node = NULL;
     while(tmp_room_node){
          if(!strcmp(tmp_room_node->id, id)){ //room id已存在
               return 1; // room has already existed;
          }
          pre_room_node = tmp_room_node;
          tmp_room_node = tmp_room_node->next;
     }
    return 0;
}