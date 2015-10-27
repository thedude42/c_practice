#include <stdio.h>
#include <string.h>

void urlify(char* str, int len) {
    int count = 0;
    char* cursor = str;
    while(*cursor != '\0') {
        if (*cursor == ' ')
            count++;
        ++cursor;
    }
    char* str2 = cursor + 2*count;
    while (count != 0) {
        if (*cursor != ' ') {
            *str2 = *cursor;
            str2--;
            cursor--;
        }
        else {
            *str2 = '0';
            str2--;
            *str2 = '2';
            str2--;
            *str2 = '%';
            str2--;
            cursor--;
            count--;
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Error: require one sting argument\n");
        return -1;
    }
    char* inarg = argv[1];
    int len =0;
    int spaces = 0;
    while (*(inarg+len) != '\0') {
        if (*(inarg+len) == ' ')
            spaces++;
        ++len;
    }
    char str[1+len+2*spaces];
    strcpy(str, inarg);
    urlify(str, len);
    printf("%s\n", str);
    return 0;
}

