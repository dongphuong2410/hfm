#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>

#include "fileutil.h"

#define BUFF_LEN 1024

static off_t _get_file_size(const char *filepath);
static int _get_checksum(const char *filepath, char *buff);
static int _cmp_file_content(const char *file1, const char *file2);

int util_checkdup(const char *file1, const char *file2)
{
    int result = 0;
    /* Check filesize */
    off_t size1 = _get_file_size(file1);
    off_t size2 = _get_file_size(file2);
    if (size1 == -1 || size2 == -1) {
        printf("Error get file size\n");
        goto done;
    }
    if (size1 != size2)
        goto done;

    /* Check file checksum */
    char cs1[BUFF_LEN] = {0}, cs2[BUFF_LEN] = {0};
    if (_get_checksum(file1, cs1) != 0) {
        printf("Error get checksum file %s\n", file1);
        goto done;
    }
    if (_get_checksum(file2, cs2) != 0) {
        printf("Error get checksum file %s\n", file2);
        goto done;
    }
    if (strcmp(cs1, cs2) != 0)
        goto done;

    /* Compare file content */
    result = (_cmp_file_content(file1, file2) == 0);
done:
    return result;
}

int util_create_symlink(const char *filename, const char *slname)
{
    struct stat st;
    if (stat(slname, &st) == 0) {
        remove(slname);
    }
    symlink(filename, slname);
    return 0;
}

static off_t _get_file_size(const char *filepath)
{
    struct stat st;
    if (stat(filepath, &st) == 0)
        return st.st_size;
    return -1;
}

static int _get_checksum(const char *filepath, char *buff)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    int i;
    FILE *fp = fopen(filepath, "rb");
    unsigned char data[BUFF_LEN];
    MD5_CTX mdContext;
    int bytes;

    if (!fp) {
        return -1;
    }
    MD5_Init(&mdContext);
    while ((bytes = fread(data, 1, BUFF_LEN, fp)) != 0) {
        MD5_Update(&mdContext, data, bytes);
    }
    MD5_Final(c, &mdContext);
    for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(buff + i * 2, "%02x", c[i]);
    }
    buff[i * 2] = '\0';
    fclose(fp);
    return 0;
}

/* Return 0 if file is same */
static int _cmp_file_content(const char *file1, const char *file2)
{
    int cmp = 0;
    FILE *fp1 = fopen(file1, "rb");
    FILE *fp2 = fopen(file2, "rb");
    if (fp1 == NULL || fp2 == NULL) {
        printf("Error open files \n");
        return 0;
    }
    unsigned char buff1[BUFF_LEN];
    unsigned char buff2[BUFF_LEN];

    int bytes1, bytes2;
    do {
        bytes1 = fread(buff1, 1, BUFF_LEN, fp1);
        bytes2 = fread(buff2, 1, BUFF_LEN, fp2);
        if (bytes1 != bytes2) {
            cmp = (bytes1 - bytes2);
            goto done;
        }
        if ((cmp = memcmp(buff1, buff2, bytes1)) != 0) {
            goto done;
        }
    } while (bytes1 > 0);
done:
    fclose(fp1);
    fclose(fp2);
    return cmp;
}
