#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

int main() {
    char* str1 = (char*) malloc(sizeof(char)*3);// = "hi";
    char str2[] = "hi";
    // str1[0] = 'i';
    strcpy(str1, "hi");
    str1[0] = 'o';
    printf("%s\n", str1);
    // printf("str1[2]=%d\nstr12[2]=%d\n", str1[2]=='\0', str2[2]=='\0');
    return 0;
}