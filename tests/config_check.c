#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "config.h"

START_TEST(test_config_module)
    int error = 1;
    char buff[1024];
    char cfgpath[1024];
    char *cwd = getcwd(buff, 1024 + 1);
    sprintf(cfgpath, "%s/%s", cwd, "sample_config.cfg");
    ck_assert(error);
END_TEST

static Suite *config_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Config Module");
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
