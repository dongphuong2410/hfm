#include <stdio.h>
#include <stdlib.h>

#include "private.h"
#include "log.h"
#include "file_filter.h"
#include "config.h"

config_t *config;

int main(int argc, char *argv)
{
    int matchno;
    filter_t *filter = filter_init();
    filter_add(filter, "/home/meo/exercise/01.pdf", 1);
    filter_add(filter, "/home/meo/exercise/02.*", 2);
    filter_add(filter, "/home/*/lesson/02.txt", 3);
    filter_add(filter, "/home/**/03.txt", 4);

    matchno = filter_match(filter, "/home/meo/exercise/01.pdf");
    printf("%d\n", matchno);
    matchno = filter_match(filter, "/home/meo/exercise/02.txt");
    printf("%d\n", matchno);
    matchno = filter_match(filter, "/home/meo/lesson/02.txt");
    printf("%d\n", matchno);
    matchno = filter_match(filter, "/home/abc/def/03.txt");
    printf("%d\n", matchno);
    matchno = filter_match(filter, "/home/abc/def/04.txt");
    printf("%d\n", matchno);
    filter_close(filter);

    filter = filter_init();
    filter_add(filter, "C:/Windows/meo/*", 10);
    matchno = filter_match(filter, "C:/Windows/meo/hello.txt");
    printf("%d\n", matchno);
}

