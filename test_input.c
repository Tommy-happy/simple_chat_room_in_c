// #include <stdio.h>
// #include <string.h>

// int main(){
//     char c;
//     while(c=getchar() != '\n'){
//         printf("%d\n", c);
//     }
// }

#include <curses.h>

int main() {
    initscr(); // 初始化ncurses库
    raw(); // 禁用行缓冲
    noecho();
    keypad(stdscr, TRUE); // 启用特殊键盘键输入，如方向键
    scrollok(stdscr, false);        // 在 WIN 指针指向的窗口中开启 (true)/关闭 (false) 滚屏
    int row,col;
    getmaxyx(stdscr, row, col);
    int ch;
    while ((ch = getch()) != '\n') { // 以F1键作为退出条件
        switch (ch) {
            case KEY_UP:
                printw("Up Arrow key pressed\n");
                break;
            case KEY_DOWN:
                printw("Down Arrow key pressed\n");
                break;
            case KEY_LEFT:
                printw("Left Arrow key pressed\n");
                break;
            case KEY_RIGHT:
                printw("Right Arrow key pressed\n");
                break;
            case KEY_BACKSPACE:
            case KEY_DC:
                printw("Delete key pressed\n");
                break;
            default:
                printw("Character pressed: %c\n", ch);
                break;
        }
        setscrreg(row, row);
        refresh();
    }

    endwin(); // 结束ncurses库
    return 0;
}