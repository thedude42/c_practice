#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
    FILE* f;
    struct stat infileStat;
   
   if (argc < 2) {
        printf("Need a file, fool\n");
        return 1;
    }
    
    if (! (f = fopen(argv[1], "r")) ) {
        printf("Could not open file named %s, errno: %d\n", argv[1], errno);
        return errno;
    }
    
    if (stat(argv[1], &infileStat)) {
        printf("can't stat file %s, errno: %d\n", argv[1], errno);
        return errno;
    }

    printf("file %s is %d bytes in size\n", argv[1], (int)infileStat.st_size);
    

}
