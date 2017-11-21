#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"

#define MAX 20

void _get_time(char *buffer);

char *MAINLOG = "hfm.log";
char *MAINTHREAD = "MAIN THREAD";
logger_t logger;
char *logdir;
level_t cur_level = LV_ERROR;
FILE **fps;
char (*names)[1024];
int entry_count;

const char *str_level[] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG"};

void log_init(level_t level, logger_t log_type, ...)
{
    logger = log_type;
    cur_level = level;
    fps = (FILE **)calloc(MAX, sizeof(FILE *));
    names = calloc(MAX, sizeof(char) * 1024);
    entry_count = 1;
    //Init main log
    if (logger == LOG_CONSOLE) {
        fps[0] = stdout;
        sprintf(names[0], "%s", MAINTHREAD);
    }
    else if (logger == LOG_TEXTFILE) {
        va_list args;
        va_start(args, log_type);
        logdir = va_arg(args, char *);
        char filepath[1024];
        sprintf(filepath, "%s/%s", logdir, MAINLOG);
        fps[0] = fopen(filepath, "a");
        if (fps[0] == NULL) {
            fprintf(stderr, "Error open log file %s\n", filepath);
        }
        va_end(args);
    }
}

int log_add_entry(const char *name)
{
    if (entry_count >= MAX) {
        writelog(0, LV_WARN, "Number of log entry exceeds the maximum (%d entries)", MAX);
        return -1;
    }
    sprintf(names[entry_count], "%s", name);
    if (logger == LOG_CONSOLE) {
        fps[entry_count] = stdout;
    }
    else {
        char filepath[1024];
        sprintf(filepath, "%s/%s.log", logdir, name);
        fps[entry_count] = fopen(filepath, "a");
        if (fps[entry_count] == NULL) {
            fprintf(stderr, "Error open log file %s\n", filepath);
            return -1;
        }
    }
    entry_count++;
    return (entry_count -1);
}

void log_close(void)
{
    int i;
    for (i = 0; i < entry_count; i++) {
        if (fps[i] && fps[i] != stdout) fclose(fps[i]);
    }
    free(fps);
    free(names);
}

void writelog(unsigned int id, level_t level, const char *message, ...)
{
    if (level > cur_level)
        return;
    char curtime[26];
    _get_time(curtime);

    FILE *fp = fps[id];
    if (!fp) fp = stdout;   //this help logger works even when no log_init called
    char *thread_name = names[id] ? names[id] : MAINTHREAD;
    /* Log to file */
    fprintf(fp, "%s ", curtime);
    fprintf(fp, "%s ", str_level[level]);
    if (fp == stdout) {
        fprintf(fp, "[%s] ", thread_name);
    }

    va_list args;
    va_start(args, message);
    vfprintf(fp, message, args);
    va_end(args);
    fprintf(fp, "\n");
}

void _get_time(char *buffer)
{
    time_t timer;
    struct tm *tm_info;

    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}
