#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include "config.h"

START_TEST(test_config_module)
{
    int error = 1;
    char buff[1024];
    char cfgpath[1024];
    char *cwd = getcwd(buff, 1024);
    sprintf(cfgpath, "%s/%s", cwd, "sample_config.cfg");
    config_t *config = config_init(cfgpath);
    int age = config_get_int(config, (const char *)"age");
    ck_assert_int_eq(config_get_int(config, (const char *)"age"), 10);
    ck_assert_str_eq(config_get_str(config, (const char *)"name"), "DongPhuong");
    ck_assert_int_eq(config_get_int(config, (const char *)"salary"), -1);
    ck_assert_ptr_null(config_get_str(config, (const char *)"school"));
}
END_TEST

static Suite *config_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Config");
    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_config_module);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = config_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
