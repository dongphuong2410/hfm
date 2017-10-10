#include <stdio.h>
#include <stdlib.h>
#include "filewatch.h"

struct _filewatch_t {
};

filewatch_t *filewatch_init(const char *path)
{
    return NULL;
}

void filewatch_set_cb(filewatch_t *fw, void (*on_recv)(const char *path, const char *real_path))
{
}

void filewatch_start(filewatch_t *fw)
{
}

void filewatch_stop(filewatch_t *fw)
{
}

void filewatch_destroy(filewatch_t *fw)
{
}
