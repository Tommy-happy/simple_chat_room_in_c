#include <ncurses.h>

#if 0
int main() {
    initscr();           // 初始化 ncurses
    scrollok(stdscr, TRUE);  // 开启标准屏幕的滚动功能

    for (int i = 0; i < 100; ++i) {
        printw("Line %d\n", i);  // 输出内容，超出屏幕时会自动滚动
        refresh();  // 更新屏幕
        napms(100); // 稍作延迟
    }

    getch();      // 等待按键
    endwin();     // 退出 ncurses
    return 0;
}
#else

int main() {
    initscr();          // 初始化 ncurses
    int max_y, max_x;   // 用于存储屏幕尺寸
    getmaxyx(stdscr, max_y, max_x);  // 获取屏幕尺寸
    keypad(stdscr, TRUE);
    box(stdscr, '|', '-');
    noecho();
    curs_set(0);
    WINDOW *pad = newpad(1000, max_x);  // 创建一个大 pad
    for (int i = 0; i < 1000; ++i) {
        wprintw(pad, "Line %d\n", i);  // 向 pad 输出内容
    }

    int pad_pos = 0;  // pad 的初始位置
    char ch;
    while ((ch = getch()) != 'q') {  // 按 'q' 退出
        if(ch==('B'&0x1f)) //key up
        {
            pad_pos++;
        }    
        else if(ch==('C'&0x1f)) //key down
        {
            pad_pos = (pad_pos > 0) ? pad_pos - 1 : 0;
        }
        else if(ch==('s')) //key up
        {
            pad_pos++;
        }
        else if(ch==('W')) //key down
        {
            pad_pos = (pad_pos > 0) ? pad_pos - 1 : 0;
        }
        // else{
        //     printw("%c cmp %c = %d",ch, 'C'&0x1f, ch==('C'&0x1f));
        // }
        prefresh(pad, pad_pos/*pad起始row*/, 0/*pad起始col*/, 1/*stdscr起始row*/, 1/*stdscr起始col*/, max_y-2/*stdscr結束row*/, max_x-2/*stdscr結束col*/);  // 刷新显示 pad 的部分
    }

    delwin(pad);       // 删除 pad
    endwin();          // 结束 ncurses
    return 0;
}

#endif