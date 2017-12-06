#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include "log.h"

START_TEST(test_log_level_debug)
{
    const char *log_file = "hfm.log";
    remove(log_file);
    log_init(LV_DEBUG, LOG_TEXTFILE, ".");
    writelog(0, LV_DEBUG, "%s", "debug level - debug log");
    writelog(0, LV_WARN, "%s", "debug level - warn log");
    writelog(0, LV_FATAL, "%s", "debug level - fatal log");
    log_close();

    FILE *fp = fopen(log_file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    ssize_t read;
    size_t len = 0;
    char *line = NULL;
    int contains;
    read = getline(&line, &len, fp);
    ck_assert_int_ne(read, -1);
    contains = strstr(line, "debug level - debug log") != NULL;
    ck_assert(contains);

    read = getline(&line, &len, fp);
    ck_assert_int_ne(read, -1);
    contains = strstr(line, "debug level - warn log") != NULL;
    ck_assert(contains);

    read = getline(&line, &len, fp);
    ck_assert_int_ne(read, -1);
    contains = strstr(line, "debug level - fatal log") != NULL;
    ck_assert(contains);

    read = getline(&line, &len, fp);
    ck_assert_int_eq(read, -1);

    fclose(fp);

    remove(log_file);
}
END_TEST

START_TEST(test_log_level_warn)
{
    const char *log_file = "hfm.log";
    remove(log_file);
    log_init(LV_WARN, LOG_TEXTFILE, ".");
    writelog(0, LV_INFO, "%s", "warn level - info log");    //Not logged
    writelog(0, LV_WARN, "%s", "warn level - warn log");    //Logged
    writelog(0, LV_ERROR, "%s", "warn level - error log");  //Logged
    log_close();

    FILE *fp = fopen(log_file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    ssize_t read;
    size_t len = 0;
    char *line = NULL;
    int contains;
    read = getline(&line, &len, fp);
    ck_assert_int_ne(read, -1);
    contains = strstr(line, "warn level - warn log") != NULL;
    ck_assert(contains);

    read = getline(&line, &len, fp);
    ck_assert_int_ne(read, -1);
    contains = strstr(line, "warn level - error log") != NULL;
    ck_assert(contains);

    read = getline(&line, &len, fp);
    ck_assert_int_eq(read, -1);

    fclose(fp);

    remove(log_file);
}
END_TEST

START_TEST(test_log_level_fatal)
{
    const char *log_file = "hfm.log";
    remove(log_file);
    log_init(LV_FATAL, LOG_TEXTFILE, ".");
    writelog(0, LV_INFO, "%s", "fatal level - info log");    //Not logged
    writelog(0, LV_WARN, "%s", "fatal level - warn log");    //Not logged
    writelog(0, LV_FATAL, "%s", "fatal level - fatal log");  //Logged
    log_close();

    FILE *fp = fopen(log_file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    ssize_t read;
    size_t len = 0;
    char *line = NULL;
    int contains;

    read = getline(&line, &len, fp);
    ck_assert_int_ne(read, -1);
    contains = strstr(line, "fatal level - fatal log") != NULL;
    ck_assert(contains);

    read = getline(&line, &len, fp);
    ck_assert_int_eq(read, -1);

    fclose(fp);

    remove(log_file);
}
END_TEST

static Suite *log_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Log");
    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_log_level_debug);
    tcase_add_test(tc_core, test_log_level_warn);
    tcase_add_test(tc_core, test_log_level_fatal);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = log_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
