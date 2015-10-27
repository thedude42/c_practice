#include <stdio.h>


int has_unique(char* str) {
    char* curr = str;
    int place = 0;
    while (*curr != '\0') {
        char* cursor = curr + 1;
        while (*cursor != '\0' && *curr != *cursor)
            cursor++;
        if (*curr == *cursor)
            return place;
        curr++;
        place++;
    }
    return -1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Error: require one sting argument\n");
        return -1;
    }
    char* inarg = argv[1];
    int n = 0;
    while (inarg[n] != '\0') {
        ++n;
    }
    int place = has_unique(inarg);
    if (place != -1)
        printf("%s has non-unique character at index %d\n", inarg, place);
    else
        printf("%s has all unique characters\n", inarg);
    return 0;
}
