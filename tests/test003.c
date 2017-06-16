#include <stdio.h>
#include <stdlib.h>

#include "private.h"
#include "log.h"
#include "file_filter.h"
#include "config.h"

config_t *config;
int interrupted;

int main(int argc, char *argv)
{
    int matches[15];
    int matchno;
    filter_t *filter = filter_init();
    filter_add(filter, "/home/meo/exercise/01.pdf", 1);
    filter_add(filter, "/home/meo/exercise/02.*", 2);
    filter_add(filter, "/home/*/exercise/02.txt", 3);
    filter_add(filter, "/home/**/02.txt", 3);

    matchno = filter_match(filter, "/home/meo/exercise/01.pdf", matches);
    printf("%d\n", matchno);
    matchno = filter_match(filter, "/home/meo/exercise/02.txt", matches);
    printf("%d\n", matchno);
    filter_close(filter);
}

