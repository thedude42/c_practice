#include <stdio.h>
#include <stdlib.h>

void reverse(int len, char* str) {
    for (len -= 1; len >= 0; --len)
        putc(str[len], stdout);
    putc('\n', stdout);
}

int isPalendrome(int len, char* str) {
    int i = 0;
    int j = len - 1;
    while (i < len && j >= 0) {
        while (str[i] == ' ')
            i++;
        while (str[j] == ' ')
            j--;
        if (str[i] != str[j])
            return 0;
        i++;
        j--;
    }
    return 1;
}


int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Error: require one sting argument\n");
        exit(1);
    }
    char* inarg = argv[1];
    int n = 0;
    while (inarg[n] != '\0') {
        ++n;
    }
    reverse(n, inarg);
    if (isPalendrome(n, inarg))
        printf("%s is a palendrome\n", inarg);
    else
        printf("%s is NOT a palendrome\n", inarg);

    return 0;
}
