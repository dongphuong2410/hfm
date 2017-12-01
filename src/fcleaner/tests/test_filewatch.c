#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "../filewatch.h"

int running;

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("<usage> %s <filename>\n", argv[0]);
        exit(0);
    }

    running = 1;
    filewatch_t *fw = filewatch_init(argv[1]);
    assert(fw != NULL);

    filewatch_start(fw);
    while (running) {
        sleep(2);
    }

    filewatch_destroy(fw);
    return 0;
}
