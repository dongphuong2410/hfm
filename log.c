#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"

void _get_time(char *buffer);

logger_t logger = LOG_CONSOLE;
FILE *fp;
const char *str_level[] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG"};

void log_init(logger_t l, ...)
{
    logger = l;
    if (l == LOG_CONSOLE) {
        fp = stdout;
    }
    else {
        va_list args;
        va_start(args, l);
        char *filepath = va_arg(args, char *);
        fp = fopen(filepath, "a");
        if (fp == NULL) {
            fprintf(stderr, "Error open log file %s\n", filepath);
        }
        va_end(args);
    }
}

void log_close(void)
{
    if (fp && fp != stdout) fclose(fp);
}

void writelog(level_t level, const char *message, ...)
{
    if (level > CURRENT_LEVEL)
        return;
    char curtime[26];
    _get_time(curtime);

    fprintf(fp, "%s ", curtime);
    fprintf(fp, "%s ", str_level[level]);

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
