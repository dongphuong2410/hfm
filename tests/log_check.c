#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include "log.h"

START_TEST(test_log_level)
{
}
END_TEST

static Suite *log_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Config");
    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_log_level);
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
