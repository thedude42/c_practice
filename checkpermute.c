#include <stdio.h>

int get_len(char* s1, char* s2) {
    int len;
    while (*s1 != '\0' && *s2 != '\0') {
        s1++;
        s2++;
        len++;
    }
    if (*s1 != '\0' || *s2 != '\0')
        return -1;
    return len;
}

int check_permute(char* s1, char* s2) {
    int len = get_len(s1, s2);
    if (len == -1) {
        printf("bail early because length mismatch\n");
        return 0;
    }
    int skip, found = 0;
    int indexes[len];
    for (int i=0; i<len; ++i) {
        for (int j=0; j<len; ++j) {
            skip = 0;
            if (s1[i] == s2[j]) {
                for (int k=0; k<found; ++k) {
                    if (indexes[k] == j) {
                        skip = 1;
                        break;
                    }
                }
                if (skip)
                    continue;
                indexes[i] = j;
                found++;
                break;
            }
        }
        if (found == i) {
            printf("bailing early because string in s1 not found in s2\nlen==%d,i==%d,indexes[i]==%d,found==%dskip==%d\n",len,i,indexes[i],found,skip);
            return 0;
        }
    }
    int sum=0, target=0;
    for (int s=0; s<len; ++s) {
        target += s;
        sum += indexes[s];
    }
    return target == sum;                   
}


int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Error: require two sting arguments\n");
        return -1;
    }
    if (check_permute(argv[1], argv[2]))
        printf("String %s is a permutation of %s\n", argv[1], argv[2]);
    else
        printf("These strings are not permutations of eachother\n");
}
