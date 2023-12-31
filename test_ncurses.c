#include <stdio.h>
#include <ncurses.h>
#include <string.h>

int get_name_room_id(char[20], char[5]);

int main(int argc, char** argv){
    initscr();
    char* str1 = "Hi Tommy";
    char* str2 = "111111111111111";
    char* str3 = "2222222222222222";
    char name[20];
    char room_id[5];
    int is_join;
    // noecho();
    keypad(stdscr, TRUE); // 启用特殊键盘键输入，如方向键
    
    int scrLine, scrCol, a;
    printw("Main Menu\n");          // 标准窗口作为主界面 main menu
    refresh();
    scr_dump("a");                  // 储存当前标准窗口的内容

    getch();
    WINDOW* win = subwin(stdscr, scrLine, scrCol, 0, 0);
    wclear(stdscr);
    wprintw(win, "Game Started!\n");     // 一个与标准窗口相同大小的新窗口 win 作为游戏界面
    wrefresh(win);

    getch();
    wclear(win);
    wrefresh(win);                  // 游戏界面结束，将屏幕清空
    delwin(win);                    // 释放游戏界面所在窗口 win 的内存

    getch();
    scr_restore("a");               // 返回主界面 main menu, 使用 scr_restore() 恢复
    getch();
    refresh();
    // getmaxyx(stdscr, scrLine, scrCol);
    // WINDOW* upwin = subwin(stdscr, scrLine / 2, scrCol, 0, 0);
    // WINDOW* downwin = subwin(stdscr, scrLine / 2, scrCol, scrLine / 2, 0);
    // box(upwin, '|', '-');
    // box(downwin, '|', '-');
 
    // mvwprintw(upwin, 1,1, "up screen");
    // mvwprintw(downwin, 1,1, "down screen");

    // getch();
    // wclear(downwin);
    // wrefresh(downwin);
    // getch();
    // box(downwin, '|', '-');
    // getmaxyx(stdscr, scrLine, scrCol);
    // mvwprintw(downwin, 1,1, "stdwin: row=%d\tcol=%d", scrLine, scrCol);
    // getmaxyx(downwin, scrLine, scrCol);
    // mvwprintw(downwin, 2,1, "down:   row=%d\tcol=%d", scrLine, scrCol);
    // // wrefresh(downwin);
    // mvwprintw(upwin, 1,1, "renew up window");
    // touchwin(stdscr); 
    // getch();
    endwin();
}

int get_name_room_id(char name[20], char room[5]){
    int row, col;
    int len=0;
    char c;
    int enter; //有沒有按enter
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
            if(len<19){
                name[len]=c;
                name[len+1]='\0';
            }
            len++;    
        }

        clear();
        mvprintw(row/2-1, (col-17)/2+1, "%s","what\'s your name?");
        if(len==20){
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


    char join_room[] = "join room"; //len=9
    char creat_room[] = "create room"; //len=11
    int is_join=1;
    clear();
    mvprintw(row/2, (col-31)/2+1, "%s","I want to ");
    attrset(A_REVERSE | A_UNDERLINE);
    mvprintw(row/2, (col-31)/2+1+10, "%s", join_room);
    attroff(A_REVERSE | A_UNDERLINE);
    mvprintw(row/2, (col-31)/2+1+19, "/%s", creat_room);
    while( (c = getch()) != '\n'){ //輸入名字
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
    mvprintw(row/2-1, (col-25)/2+1, "%s","please input the room id?");
    move(row/2, (col-len)/2);
    while( (c = getch()) ){ //輸入room id
        enter = 0;
        if(c == ('G'&0x1f)){ //輸入刪除鍵
            if(len==0){
                continue;
            }
            room[len-1]='\0';
            len--;
        }
        else if(c=='\n'){
            if(len!=4){
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
            if(len<4){
                room[len]=c;
                room[len+1]='\0';
            }
            len++;    
        }

        clear();
        mvprintw(row/2-1, (col-25)/2+1, "%s","please input the room id?");
        if(len==5){
            // room[4]='\0';
            mvprintw(row/2+1, (col-25)/2+1, "no more than four numbers");
            len--;
        }
        mvprintw(row/2, (col-len)/2, "<%s>",room);
        if(enter){
            mvprintw(row/2+1, (col-32)/2+1, "please input exacly four numbers");
        }
        refresh();
    }

    return is_join;
}