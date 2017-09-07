#include <stdio.h>
#include <stdlib.h>

#include "private.h"
#include "log.h"
#include "output_format.h"
#include "config.h"
#include "policy.h"

config_t *config;

int main(int argc, char *argv)
{
    char *csvpath = "./tmp.csv";
    output_info_t info;
    //strcpy(info.time, "2017-06-02 03:14");
    info.time_sec = 15000145;
    info.time_usec = 1234;
    info.pid = 1120;
    info.vmid = 20;
    info.policy_id = 30;
    info.action = MON_DELETE;
    strcpy(info.filepath, "/bin/lib/abc.so");
    strcpy(info.extpath, "/home/phuong/abc.so");
    strcpy(info.data, "Custom data");

    output_t *out = out_init(OUT_CONSOLE);
    out_write(out, &info);
    out_close(out);

    out = out_init(OUT_CSV, csvpath);
    out_write(out, &info);
    out_close(out);

    FILE *fp = fopen(csvpath, "r");
    char c;
    char str[1024];
    int cnt = 0;
    if (fp) {
        while ((c = getc(fp)) != '\r' && c != '\n' && c != EOF)
            str[cnt++] = c;
    }
    fclose(fp);
    str[cnt] = '\0';
    printf("%s\n", str);

    remove(csvpath);
    return 0;
}

