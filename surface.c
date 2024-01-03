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

int messnum = 0; // ���ثe��X�����(pad)
int roll = 0;    // �w��u�����ʡA���v�Tmessnum�A�@��X�N�|��_�쪬=0

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
    mvwprintw(pad, messnum, 1, "%s\n", name); // �V pad ?�Xname(���]name���פ��W�L�����A���M�U�@��n�հѼ�)
    messnum++;
    wattroff(pad, COLOR_PAIR(1));
    wrefresh(pad);
    if (strlen(mess) > max_x * 0.6)
    {
        int len = max_x * 0.6;
        for (i = 0; i < strlen(mess); i++)
        {
            j = i % len;
            mvwprintw(pad, i / len + messnum, 1 + j + tab, "%c", mess[i]); // j�C����X���k ; i���W�L�@�橹�U����
            /// pad�_�lrow//pad�_�lcol//stdscr�_�lrow//stdscr�_�lcol/stdscr����row/stdscr����col/
        }
        messnum += strlen(mess) / len + 1;
    }
    else
    {
        mvwprintw(pad, messnum, 1 + tab, "%s", mess);
        messnum++;
    }
    if (messnum > max_y - 3)
    { // �~�ؤ��i�e��23+1(�w��)��(max_y=26)
        prefresh(pad, messnum - (max_y - 3), 0, 1, 1, max_y - 3, max_x - 2);
    }
    else
    {
        prefresh(pad, 0, 0, 1, 1, max_y - 3, max_x - 2); // ��s?�� pad ������
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
            mvwprintw(pad, i / len + messnum, max_x - len + j, "%c", chpre[i]); // �V pad ?�X?�e
            mvwprintw(pad, i / len + messnum, 0, "%d", messnum);
            // prefresh(pad, roll, 0, 1, 1, max_y - 2, max_x - 2);
            //  wrefresh(pad);
        }
        messnum += charnum / len + 1;
        /// pad�_�lrow//pad�_�lcol//stdscr�_�lrow//stdscr�_�lcol/stdscr����row/stdscr����col/
    }
    else
    {
        // wattron(pad, COLOR_PAIR(1));
        mvwprintw(pad, messnum, max_x - charnum - 2, "%s", chpre);
        messnum++;
        // wattroff(pad, COLOR_PAIR(1));
    }
    if (messnum > max_y - 3)
    { // �~�ؤ��i�e��23+1(�w��)��(max_y=26)
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
    char chpre[max_x]; // �̤j�榸��X�W��
    start_color();
    init_pair(2, COLOR_BLUE, COLOR_WHITE); // �w�]�n�C���
    init_pair(3, COLOR_WHITE, COLOR_WHITE);
    attron(COLOR_PAIR(3));                  // �}���C���3����off
    mvhline(max_y - 2, 0, ACS_HLINE, COLS); // �b���U�@��ø�s����?
    attroff(COLOR_PAIR(3));
    while ((c = getch()) != 27)
    {
        if (c == ('B' & 0x1f) || c == ('C' & 0x1f))
        {
            if (c == ('B' & 0x1f) && roll + messnum - (max_y - 3) > 0) // key up
            {
                roll--;
            }
            else if (c == ('C' & 0x1f) && roll < 0) // key down(�����W�Ƥ~�੹�U)
            {
                roll++;
            }
            if (messnum > max_y - 3) // �~�ؤ��i�e��23+1(�w��)��(max_y=26)
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
            mvprintw(max_y - 2, inptcharnum, "%c", c); // �V stdscr ?�X?�e
            attroff(COLOR_PAIR(2));
            refresh();
        }
        // else if (c==127){       //delete��{���F
        //     if (inptcharnum >0 )
        //     {
        //         chpre[inptcharnum-1] = '\0';
        //         attron(COLOR_PAIR(4));
        //         mvprintw(max_y - 2, inptcharnum, " "); // �V stdscr ?�X?�e
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
            chpre[inptcharnum] = '\0'; // �Φ��r��***
            if (memcmp(chpre, "$$CR", 4) == 0)
            {
                string cmd = strtok(chpre, " ");
                string roomaddr = strtok(NULL, "");
                // mvprintw(max_y /2, max_x/2, "create room" ); // ?�X?�etest
                // prefresh(pad, max_y /2, max_x/2, 1, 1, max_y - 3, max_x - 2);
            }
            else if (memcmp(chpre, "$$OR", 4) == 0)
            {
                string cmd = strtok(chpre, " ");
                string roomaddr = strtok(NULL, "");
                // mvwprintw(pad, max_y /2, max_x/2, "open room" );//?�X?�etest
                // prefresh(pad, max_y /2, max_x/2, 1, 1, max_y - 3, max_x - 2);
                // roomaddr=XXXX
            }
            mymessageoutput(pad, chpre, inptcharnum);
            attron(COLOR_PAIR(3));
            mvhline(max_y - 2, 0, ACS_HLINE, COLS); // �b���U�@��ø�����?
            attroff(COLOR_PAIR(3));
            inptcharnum = 0; // �w����r���s�p��
            roll = 0;        // ��_�쪬
        }
    }
}

int main()
{
    int i, num = 0;
    initscr();
    keypad(stdscr, TRUE); // �ҥίS����L��J�A�p��V��
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    noecho();
    curs_set(0);
    WINDOW *pad = newpad(1000, max_x); // �Ыؤ@�Ӥj pad
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
    // pad = othermessageoutput(str, pad); //����X
    // pad = othermessageoutput(str1, pad);
    // pad = othermessageoutput(str2, pad);
    // pad = othermessageoutput(str3, pad);
    // pad = othermessageoutput(str4, pad);
    // pad = othermessageoutput(str5, pad);
    // pad = othermessageoutput(str6, pad);
    // pad = othermessageoutput(str7, pad);
    // pad = othermessageoutput(str8, pad);
    preview(pad); // �ۤv��X+�w��
    // pad = othermessageoutput(str9, pad);
    getch();
    endwin();
    return 0;
}
