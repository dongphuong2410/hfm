/**
  * @brief Unit test for config.c file
  *
  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

int main(int argc, char **argv)
{
    char *cwd;
    char buff[1024];

    cwd = getcwd(buff, 1024 + 1);
    char cfgpath[1024];
    sprintf(cfgpath, "%s/%s", cwd, "test001.cfg");
    config_t *config = config_init(cfgpath);

    int age = config_get_int(config, (const char *)"age");
    printf("%d\n", age);
    char *name = config_get_str(config, (const char *)"name");
    printf("%s\n", name);
    int salary = config_get_int(config, (const char *)"salary");
    printf("%d\n", salary);
    char *school = config_get_str(config, (const char *)"school");
    if (school)
        printf("%s\n", "NOT NULL");
    else
        printf("%s\n", "NULL");
    config_close(config);
    return 0;
}


