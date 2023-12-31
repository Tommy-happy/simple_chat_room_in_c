#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <omp.h>
#include <ncurses.h>

#define BUFFER_SIZE 256
#define USERNAME_LEN 19
#define ROOM_ID_LEN 4

void get_name(char name[USERNAME_LEN+1]);
int get_room_id(char[ROOM_ID_LEN+1]);

void error(const char *msg) { //輸出錯誤訊息
    perror(msg); //輸出msg和stderr的內容
    exit(0); //結束程式
}

int main(int argc, char *argv[]) {
    int serverfd /*socket file description*/, portno, n, is_join, col, row;
    struct sockaddr_in serv_addr; //server地址等資訊
    struct hostent *server;
    char username[USERNAME_LEN+1], room_id[ROOM_ID_LEN+1];
    char is_join_str[2];

    char in_buffer[BUFFER_SIZE];
    char out_buffer[BUFFER_SIZE];
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

    initscr(); //初始化ncurses視窗
    noecho(); //輸入時terminal不會顯示字元
    keypad(stdscr, TRUE); // 启用特殊键盘键输入，如方向键
    // curs_set(0); //不顯示屬標

    get_name(username);
    n = write(serverfd,username,strlen(username)); //傳送給server
    if (n < 0)  error("ERROR writing to socket");
    // printf("sended %s\n", username);
    sleep(0.1);

    type_room_id:{}
    is_join = get_room_id(room_id); //取得client username、創建或加入房間、room id
    // mvprintw(0,0,"sended %s\n", room_id);
    sprintf(is_join_str,"%d", is_join);
    n = write(serverfd,is_join_str,1); //傳送給server
    if (n < 0)  error("ERROR writing to socket");
    sleep(0.1);
    // mvprintw(0, 0, "is_join=%d\nis_join_str=%s", is_join, is_join_str);

    n = write(serverfd,room_id,strlen(room_id)+1); //傳送給server
    if (n < 0)  error("ERROR writing to socket");
    // printf("sended %s\n", is_join_str);

    //等待server回應加入房間狀況或是創建房間狀況
    n = read(serverfd,in_buffer,BUFFER_SIZE); //卡在這裡直到收到伺服端的輸入
    if (n < 0)  error("ERROR reading from socket");
    if(strncmp(in_buffer, "200OK", 5)){ //有錯

        getmaxyx(stdscr, row, col);
        if(is_join == 1){
            mvprintw(row/2, (col-22)/2+1, "the room doesn't exist");
        }
        else{
            // mvprintw( 0, 0, "%s", in_buffer);
            mvprintw(row/2, (col-27)/2+1, "the room is already existed");
        }
        mvprintw(row/2+1, (col-35)/2+1, "please press any key to continue...");
        getch();
        clear();
        goto type_room_id;
    }
    endwin(); //結束ncurses視窗

    omp_set_num_threads(2); //設定平行運算thread的數量
    #pragma omp parallel /*開始平行運算*/ sections //根據section平行運算，section1和section2同時進行 
    {
        #pragma omp section //section1
        {    
            // printf("waiting for you to write something...\n");
            while(fgets(in_buffer,BUFFER_SIZE-1,stdin)!=NULL){ //讀取std輸入
                if(strlen(in_buffer) == 1){
                    continue;
                }
                n = write(serverfd,in_buffer,strlen(in_buffer)); //傳送給server
                if (n < 0)  error("ERROR writing to socket");
                // printf("I wrote something...\n");
            }
        }
        #pragma omp section//section2
        {    
            while(1){ //讀取server輸出
                bzero(out_buffer,BUFFER_SIZE-1);
                // printf("recieving... \n");
                n = read(serverfd,out_buffer,BUFFER_SIZE-1); //卡在這裡直到收到伺服端的輸入
                if (n < 0)  error("ERROR reading from socket");
                else if(n == 0) break;
                // printf("recieved\n");
                printf("%s",out_buffer);
            }
        }
    }

    close(serverfd);
    return 0;
}

void get_name(char name[USERNAME_LEN+1]){
    int row, col;
    int len=0;
    char c;
    int enter; //有沒有按enter
    bzero(name, USERNAME_LEN+1);
    getmaxyx(stdscr, row, col);  //(ncurses)得目前terminal的長和寬
    mvprintw(row/2-1, (col-17)/2,"%s","what\'s your name?"); //移動並輸出
    move(row/2, (col-len)/2);
    while(c = getch()){ //輸入名字
        int enter=0;
        if(c == ('G'&0x1f)){ //輸入刪除鍵
            if(len==0){
                continue;
            }
            name[len-1]='\0';
            len--;
        }
        else if(c=='\n'){
            if(len>0){
                break;
            }
            else{
                enter = 1;
            }
        }
        else if(c<32 || 126<c){
            continue;
        }
        // else if(c == ('C'&0x1f)){
        //     strcpy(name, "you pressed up");
        //     break;
        // }
        // else if(c == ('B'&0x1f)){
        //     strcpy(name, "you pressed down");
        //     break;
        // }
        // else if(c == ('E'&0x1f)){
        //     strcpy(name, "you pressed right");
        //     break;
        // }
        // else if(c == ('D'&0x1f)){
        //     strcpy(name, "you pressed left");
        //     break;
        // }
        else{
            if(len<USERNAME_LEN){
                name[len]=c;
                name[len+1]='\0';
            }
            len++;    
        }

        clear();
        mvprintw(row/2-1, (col-17)/2+1, "%s","what\'s your name?");
        if(len==USERNAME_LEN+1){
            move(row/2+1, (col-16)/2+1);
            printw("name is too long");
            len--;
        }
        else if(enter){
            mvprintw(row/2+1, (col-22)/2+1, "please input your name");
        }
        mvprintw(row/2, (col-len)/2, "<%s>",name);
        refresh();
    }
    clear();
}

int get_room_id(char room_id[ROOM_ID_LEN+1]){
    int row, col;
    int len=0;
    char c;
    int enter; //有沒有按enter
    
    getmaxyx(stdscr, row, col);  //(ncurses)得目前terminal的長和寬

    char join_room[] = "join room"; //len=9
    char creat_room[] = "create room"; //len=11
    int is_join=1;
    clear();
    mvprintw(row/2, (col-31)/2+1, "%s","I want to ");
    attrset(A_REVERSE | A_UNDERLINE);
    mvprintw(row/2, (col-31)/2+1+10, "%s", join_room);
    attroff(A_REVERSE | A_UNDERLINE);
    mvprintw(row/2, (col-31)/2+1+19, "/%s", creat_room);
    while( (c = getch()) != '\n'){ //選擇是否join
        if(c == ('D'&0x1f)){ //press left
            clear();
            mvprintw(row/2, (col-31)/2+1, "%s","I want to ");
            attrset(A_REVERSE | A_UNDERLINE);
            mvprintw(row/2, (col-31)/2+1+10, "%s", join_room);
            attroff(A_REVERSE | A_UNDERLINE);
            mvprintw(row/2, (col-31)/2+1+19, "/%s", creat_room);
            is_join = 1;
        }
        else if(c == ('E'&0x1f)){ //press right
            clear();
            mvprintw(row/2, (col-31)/2+1, "%s","I want to ");
            mvprintw(row/2, (col-31)/2+1+10, "%s/", join_room);
            attrset(A_REVERSE | A_UNDERLINE);
            mvprintw(row/2, (col-31)/2+1+20, "%s", creat_room);
            attroff(A_REVERSE | A_UNDERLINE);
            is_join = 0;
        }
        else{
            continue;
        }
    }

    len = 0;
    enter=0;
    clear();
    bzero(room_id, ROOM_ID_LEN+1);
    mvprintw(row/2-1, (col-24)/2+1, "%s","please input the room id");
    move(row/2, (col-len)/2);
    while( (c = getch()) ){ //輸入room id
        enter = 0;
        if(c == ('G'&0x1f)){ //輸入刪除鍵
            if(len==0){
                continue;
            }
            room_id[len-1]='\0';
            len--;
        }
        else if(c=='\n'){
            if(len!=ROOM_ID_LEN){
                enter = 1;
            }
            else{
                break;
            }
        }
        else if(c<48 || 57<c){
            continue;
        }
        else{
            if(len<ROOM_ID_LEN){
                room_id[len]=c;
                room_id[len+1]='\0';
            }
            len++;    
        }

        clear();
        mvprintw(row/2-1, (col-25)/2+1, "%s","please input the room id?");
        if(len==ROOM_ID_LEN+1){
            // room[4]='\0';
            mvprintw(row/2+1, (col-25)/2+1, "no more than four numbers");
            len--;
        }
        if(len){
            mvprintw(row/2, (col-len)/2, "<%s>",room_id);
        }
        if(enter){
            mvprintw(row/2+1, (col-32)/2+1, "please input exacly four numbers");
        }
        refresh();
    }
    clear();

    return is_join;
}