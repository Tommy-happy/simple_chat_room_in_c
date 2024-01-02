#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
int messnum = 0;
typedef char *string;
WINDOW *mymessageoutput(WINDOW *, string, int);
WINDOW *othermessageoutput(string, WINDOW *, int);
void buildscreen();
void preview(WINDOW *);
typedef struct statement state;
void buildscreen()
{
    int i, j, tmp, c;
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    box(stdscr, '|', '-');
    wrefresh(stdscr);
}

WINDOW *othermessageoutput(string str, WINDOW *pad, int messnum)
{
    // Mess mdata=NULL;
    int max_y, max_x, i, j, padpos = 2;
    getmaxyx(stdscr, max_y, max_x);
    int roll = messnum - max_y + 5;
    string name = strtok(str, ":"); // name<=20-1   mess<=256-1
    string mess = strtok(NULL, "");
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_GREEN);
    wattron(pad, COLOR_PAIR(1));
    mvwprintw(pad, messnum, 1, "%s\n", name); //  pad ??甧
    wattroff(pad, COLOR_PAIR(1));
    wrefresh(pad);
    if (strlen(mess) > max_x * 0.6)
    {
        int len = max_x * 0.6;
        for (i = 0; i < strlen(mess); i++)
        {
            j = i % len;
            mvwprintw(pad, 1 + i / len + messnum, 1 + j + 3, "%c", mess[i]);
            /// pad癬﹍row//pad癬﹍col//stdscr癬﹍row//stdscr癬﹍col/stdscr挡row/stdscr挡col/
        }
        prefresh(pad, roll, 0, 1, 1, max_y - 2, max_x - 2);
    }
    else
    {
        mvwprintw(pad, 1 + messnum, 3, "%s", mess);
        prefresh(pad, roll, 0, 1, 1, max_y - 2, max_x - 2); // 穝?ボ pad 场だ
    }
    return pad;
}

WINDOW *mymessageoutput(WINDOW *pad, string chpre, int charnum)
{
    int max_y, max_x, i, j;
    getmaxyx(stdscr, max_y, max_x);

    int roll = messnum - max_y + 5;
    if (roll < 0)
    {
        roll = 0;
    }
    noecho();
    curs_set(0);
    // start_color();
    // init_pair(1, 156, 6);
    int len = max_x * 0.6;
    if (charnum > len)
    {
        for (i = 0; i < charnum; i++)
        {
            j = i % len;
            // wattron(pad, COLOR_PAIR(1));

            mvwprintw(pad, 1 + i / len + messnum, max_x - len + j, "%c", chpre[i]); //  pad ??甧
            prefresh(pad, roll, 0, 1, 1, max_y - 2, max_x - 2);
            // wrefresh(pad);
        }
        prefresh(pad, roll, 0, 1, 1, max_y - 3, max_x - 2);
        /// pad癬﹍row//pad癬﹍col//stdscr癬﹍row//stdscr癬﹍col/stdscr挡row/stdscr挡col/
        messnum += charnum / len;
    }
    else
    {
        // wattron(pad, COLOR_PAIR(1));
        mvwprintw(pad, 2 + messnum, 10, "%s", chpre);
        messnum++;
        // wattroff(pad, COLOR_PAIR(1));
        prefresh(pad, roll, 0, 1, 1, max_y - 3, max_x - 2);
    }
    return pad;
}

void preview(WINDOW *pad)
{
    int max_y, max_x, i, j, inptcharnum = 0, messnum;
    getmaxyx(stdscr, max_y, max_x);
    int len = max_x * 0.6;
    char c;
    noecho();
    curs_set(0);
    char chpre[max_x];
    int k;
    start_color();
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, COLOR_WHITE);
    attron(COLOR_PAIR(3));
    mvhline(max_y - 2, 0, ACS_HLINE, COLS); // ┏︽?キ?
    attroff(COLOR_PAIR(3));
    while ((c = getch()) != 27)
    {
        if (c == ('B' & 0x1f)) // key up
        {
            // prefresh(pad, roll, 0, 1, 1, max_y - 3, max_x - 2);
            continue;
        }
        else if (c == ('C' & 0x1f)) // key down
        {
            // prefresh(pad, roll, 0, 1, 1, max_y - 3, max_x - 2);
            continue;
        }
        else if (c == ('$'))
        {
            char ch[3];
            k = 3;
            printw("rr");
            while (k--)
            {
                ch[3 - k] = getch();
                // print char at screen
            }
            /*if (memcmp(ch, "$CR") == 0)
            {
                char tmp=getch(); //space
                char buffer[5];
                fgets(buffer, 4, stdin);
                // read from server
            }
            else if (memcmp(ch, "$OR") == 0)
            {
                char tmp=getch(); //space
                char buffer[5];
                fgets(buffer, 4, stdin);

                // open room
            }*/
        }
        else if (c >= 32 && c <= 126)
        {
            if (inptcharnum < max_x - 2)
            {
                chpre[inptcharnum] = c;
                inptcharnum++;
            }
            attron(COLOR_PAIR(2));
            mvprintw(max_y - 2, inptcharnum + 1, "%c", c); //  stdscr ??甧
            attroff(COLOR_PAIR(2));
            refresh();
        }
        else if (c == '\n')
        {
            if (inptcharnum == 0)
            {
                continue;
            }
            chpre[inptcharnum] = '\0';
            mymessageoutput(pad, chpre, inptcharnum);
            attron(COLOR_PAIR(3));
            mvhline(max_y - 2, 0, ACS_HLINE, COLS); // ┏︽?キ?
            attroff(COLOR_PAIR(3));
            int tmp = max_x;
            while (tmp--)
            {
                // mvprintw(max_y - 2, tmp, "A");
                mvwhline(stdscr, max_y - 2, tmp, ' ', COLS);
            }
            inptcharnum = 0;
        }
    }
}

int main()
{
    int i, num = 0;
    initscr();
    keypad(stdscr, TRUE); // 币ノ疭龄絃块よ龄
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    noecho();
    curs_set(0);
    WINDOW *pad = newpad(1000, max_x); // 承 pad
    buildscreen();
    // char str[] = "asdfghjkl:1dsKUUsyyyyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // // char str[] = "asdfghjkl:sdsKUUsyyyyyyyyff";
    // pad = othermessageoutput(str, pad, 0); // ⊿快猭pad癟
    // char str1[] = "asdfghjkl:2sdsKUUsyyyyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str1, pad, 3); // ⊿快猭pad癟
    // char str2[] = "asdfghjkl:3sdsKUUsyioyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str2, pad, 6); // ⊿快猭pad癟
    // char str3[] = "asdfghjkl:4sdsKUUspoyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str3, pad, 9); // ⊿快猭pad癟
    // char str4[] = "asdfghjkl:5sdsKUUssfdyyyyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str4, pad, 12); // ⊿快猭pad癟
    // char str5[] = "asdfghjkl:6dsKUUsydfyyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str5, pad, 15); // ⊿快猭pad癟
    // char str6[] = "asdfghjkl:7dsKasssdffdyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str6, pad, 18); // ⊿快猭pad癟
    // char str7[] = "asdfghjkl:8dsKasssdffdyfddfffffffffffffffffffffffffffffffffffffffgssdsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str7, pad, 21); // ⊿快猭pad癟
    // char str8[] = "asdfghjkl:9dsKasssasddddddddddddddddddddddddddddddddddddddddddddsvsdvsdvdsvfsbsfbsfbfsdbfyhgfhgfngfhttttsdf";
    // pad = othermessageoutput(str8, pad, 24); // ⊿快猭pad癟
    preview(pad);
    getch();
    endwin();
    return 0;
}
