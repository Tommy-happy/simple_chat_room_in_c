#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#define tab 3
typedef char *string;
WINDOW *mymessageoutput(WINDOW *, string, int);
WINDOW *othermessageoutput(string, WINDOW *);
void buildscreen();
void preview(WINDOW *);

int messnum = 0; // 為目前輸出的行數(pad)
int roll = 0;    // 針對滾輪移動，不影響messnum，一輸出就會恢復原狀=0

void buildscreen()
{
    int i, j, tmp, c;
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    box(stdscr, '|', '-');
    wrefresh(stdscr);
}

WINDOW *othermessageoutput(string str, WINDOW *pad)
{
    int max_y, max_x, i, j;
    getmaxyx(stdscr, max_y, max_x);
    string name = strtok(str, ":"); // name<=20-1   mess<=256-1
    string mess = strtok(NULL, "");
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_GREEN);
    wattron(pad, COLOR_PAIR(1));
    mvwprintw(pad, messnum, 1, "%s\n", name); // 向 pad ?出name(假設name長度不超過頁面，不然下一行要調參數)
    messnum++;
    wattroff(pad, COLOR_PAIR(1));
    wrefresh(pad);
    if (strlen(mess) > max_x * 0.6)
    {
        int len = max_x * 0.6;
        for (i = 0; i < strlen(mess); i++)
        {
            j = i % len;
            mvwprintw(pad, i / len + messnum, 1 + j + tab, "%c", mess[i]); // j每次輸出往右 ; i為超過一行往下換行
            /// pad起始row//pad起始col//stdscr起始row//stdscr起始col/stdscr結束row/stdscr結束col/
        }
        messnum += strlen(mess) / len + 1;
    }
    else
    {
        mvwprintw(pad, messnum, 1 + tab, "%s", mess);
        messnum++;
    }
    if (messnum > max_y - 3)
    { // 外框內可容納23+1(預覽)行(max_y=26)
        prefresh(pad, messnum - (max_y - 3), 0, 1, 1, max_y - 3, max_x - 2);
    }
    else
    {
        prefresh(pad, 0, 0, 1, 1, max_y - 3, max_x - 2); // 刷新?示 pad 的部分
    }
    return pad;
}

WINDOW *mymessageoutput(WINDOW *pad, string chpre, int charnum)
{
    int max_y, max_x, i, j;
    getmaxyx(stdscr, max_y, max_x);
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, 156, 6);
    int len = max_x * 0.6;
    wattron(pad, COLOR_PAIR(1));
    if (charnum > len)
    {
        for (i = 0; i < charnum; i++)
        {
            j = i % len;
            mvwprintw(pad, i / len + messnum, max_x - len + j, "%c", chpre[i]); // 向 pad ?出?容
            mvwprintw(pad, i / len + messnum, 0, "%d", messnum);
            // prefresh(pad, roll, 0, 1, 1, max_y - 2, max_x - 2);
            //  wrefresh(pad);
        }
        messnum += charnum / len + 1;
        /// pad起始row//pad起始col//stdscr起始row//stdscr起始col/stdscr結束row/stdscr結束col/
    }
    else
    {
        // wattron(pad, COLOR_PAIR(1));
        mvwprintw(pad, messnum, max_x - charnum - 2, "%s", chpre);
        messnum++;
        // wattroff(pad, COLOR_PAIR(1));
    }
    if (messnum > max_y - 3)
    { // 外框內可容納23+1(預覽)行(max_y=26)
        prefresh(pad, messnum - (max_y - 3), 0, 1, 1, max_y - 3, max_x - 2);
    }
    else
    {
        prefresh(pad, 0, 0, 1, 1, max_y - 3, max_x - 2);
    }

    wattroff(pad, COLOR_PAIR(1));
    return pad;
}

void preview(WINDOW *pad)
{
    int max_y, max_x, i, j, k, inptcharnum = 0;
    getmaxyx(stdscr, max_y, max_x);
    int len = max_x * 0.6;
    char c;
    noecho();
    curs_set(0);
    char chpre[max_x]; // 最大單次輸出上限
    start_color();
    init_pair(2, COLOR_BLUE, COLOR_WHITE); // 預設好顏色對
    init_pair(3, COLOR_WHITE, COLOR_WHITE);
    attron(COLOR_PAIR(3));                  // 開啟顏色對3直到off
    mvhline(max_y - 2, 0, ACS_HLINE, COLS); // 在底下一行繪製水平?
    attroff(COLOR_PAIR(3));
    while ((c = getch()) != 27)
    {
        if (c == ('B' & 0x1f) || c == ('C' & 0x1f))
        {
            if (c == ('B' & 0x1f) && roll + messnum - (max_y - 3) > 0) // key up
            {
                roll--;
            }
            else if (c == ('C' & 0x1f) && roll < 0) // key down(有往上滑才能往下)
            {
                roll++;
            }
            if (messnum > max_y - 3) // 外框內可容納23+1(預覽)行(max_y=26)
            {
                prefresh(pad, roll + messnum - (max_y - 3), 0, 1, 1, max_y - 3, max_x - 2);
            }
            else
            {
                prefresh(pad, roll, 0, 1, 1, max_y - 3, max_x - 2);
            }
        }
        else if (c >= 32 && c <= 126)
        {
            if (inptcharnum < max_x)
            {
                chpre[inptcharnum] = c;
                inptcharnum++;
            }
            attron(COLOR_PAIR(2));
            mvprintw(max_y - 2, inptcharnum, "%c", c); // 向 stdscr ?出?容
            attroff(COLOR_PAIR(2));
            refresh();
        }
        // else if (c==127){       //delete實現不了
        //     if (inptcharnum >0 )
        //     {
        //         chpre[inptcharnum-1] = '\0';
        //         attron(COLOR_PAIR(4));
        //         mvprintw(max_y - 2, inptcharnum, " "); // 向 stdscr ?出?容
        //         attroff(COLOR_PAIR(4));
        //         inptcharnum--;
        //     }
        // }
        else if (c == '\n')
        {
            if (inptcharnum == 0)
            {
                continue;
            }
            chpre[inptcharnum] = '\0'; // 形成字串***
            if (memcmp(chpre, "$$CR", 4) == 0)
            {
                string cmd = strtok(chpre, " ");
                string roomaddr = strtok(NULL, "");
                // mvprintw(max_y /2, max_x/2, "create room" ); // ?出?容test
                // prefresh(pad, max_y /2, max_x/2, 1, 1, max_y - 3, max_x - 2);
            }
            else if (memcmp(chpre, "$$OR", 4) == 0)
            {
                string cmd = strtok(chpre, " ");
                string roomaddr = strtok(NULL, "");
                // mvwprintw(pad, max_y /2, max_x/2, "open room" );//?出?容test
                // prefresh(pad, max_y /2, max_x/2, 1, 1, max_y - 3, max_x - 2);
                // roomaddr=XXXX
            }
            mymessageoutput(pad, chpre, inptcharnum);
            attron(COLOR_PAIR(3));
            mvhline(max_y - 2, 0, ACS_HLINE, COLS); // 在底下一行繪制水平?
            attroff(COLOR_PAIR(3));
            inptcharnum = 0; // 預覽文字重新計算
            roll = 0;        // 恢復原狀
        }
    }
}

int main()
{
    int i, num = 0;
    initscr();
    keypad(stdscr, TRUE); // 啟用特殊鍵盤輸入，如方向鍵
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    noecho();
    curs_set(0);
    WINDOW *pad = newpad(1000, max_x); // 創建一個大 pad
    buildscreen();
    // 1dsKUUsyyyyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsb
    // char str[] = "asdfghjkl:0sdsKUUsyyyyyyyyff";
    // char str1[] = "asdfghjkl:1dsKUUsyyyyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // char str2[] = "asdfghjkl:2sdsKUUsyyyyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // char str3[] = "asdfghjkl:3sdsKUUsyioyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // char str4[] = "asdfghjkl:4sdsKUUspoyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // char str5[] = "asdfghjkl:5sdsKUUssfdyyyyyfddfffffsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // char str6[] = "asdfghjkl:6dsKUUsydfyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // char str7[] = "asdfghjkl:7dsKasssdfhttttsdf";
    // char str8[] = "asdfghjkl:8dsKasssdffdyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // char str9[] = "asdfghjkl:9dsKassssddddddddddddddddddddddddddddddddddddddddddddsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str, pad); //對方輸出
    // pad = othermessageoutput(str1, pad);
    // pad = othermessageoutput(str2, pad);
    // pad = othermessageoutput(str3, pad);
    // pad = othermessageoutput(str4, pad);
    // pad = othermessageoutput(str5, pad);
    // pad = othermessageoutput(str6, pad);
    // pad = othermessageoutput(str7, pad);
    // pad = othermessageoutput(str8, pad);
    preview(pad); // 自己輸出+預覽
    // pad = othermessageoutput(str9, pad);
    getch();
    endwin();
    return 0;
}
