#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../fileutil.h"


int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("Please enter filename and symlink name\n");
        exit(0);
    }
    char filename[1024];
    char symlink[1024];
    strcpy(filename, argv[1]);
    strcpy(symlink, argv[2]);
    util_create_symlink(filename, symlink);
    return 0;
}

