#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "filewatch.h"

struct _filewatch_t {
    FILE *fp;
    void (*on_recv)(const char *line);
    int stop;
};

void *_watch_file(void *arg);

filewatch_t *filewatch_init(const char *path)
{
    filewatch_t *fw = (filewatch_t *)calloc(1, sizeof(filewatch_t));
    if (!fw) {
        printf("Cannot allocate memory for filewatch\n");
        goto done;
    }
    fw->fp = fopen(path, "r");
    if (!fw->fp) {
        printf("Cannot open file %s\n", path);
        free(fw);
        fw = NULL;
        goto done;
    }
done:
    return fw;
}

void filewatch_set_cb(filewatch_t *fw, void (*on_recv)(const char *line))
{
    fw->on_recv = on_recv;
}

void filewatch_start(filewatch_t *fw)
{
    pthread_t tid;
    if (!fw)
        return;
    int err = pthread_create(&tid, NULL, _watch_file, fw);
}

void filewatch_stop(filewatch_t *fw)
{
    fw->stop = 1;
}

void filewatch_destroy(filewatch_t *fw)
{
    fclose(fw->fp);
    free(fw);
}

void *_watch_file(void *arg)
{
    filewatch_t *fw = (filewatch_t *)arg;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    size_t latest_pos = ftell(fw->fp);

    while (!fw->stop) {
        printf("post %lu\n", latest_pos);
        fseek(fw->fp, latest_pos, SEEK_SET);
        read = getline(&line, &len, fw->fp);
        if (read != -1) {
            printf("Read line %s", line);
            latest_pos = ftell(fw->fp);
        }
        sleep(1);
    }
}
