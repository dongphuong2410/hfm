#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include "file_filter.h"

START_TEST(test_file_filter)
{
    int matchno;
    filter_t *filter = filter_init();
    filter_add(filter, "/home/meo/exercise/01.pdf", 1);
    filter_add(filter, "/home/meo/exercise/02.*", 2);
    filter_add(filter, "/home/*/lesson/02.txt", 3);
    filter_add(filter, "/home/**/03.txt", 4);

    matchno = filter_match(filter, "/home/meo/exercise/01.pdf");
    ck_assert_int_eq(matchno, 1);
    matchno = filter_match(filter, "/home/meo/exercise/02.txt");
    ck_assert_int_eq(matchno, 2);
    matchno = filter_match(filter, "/home/meo/lesson/02.txt");
    ck_assert_int_eq(matchno, 3);
    matchno = filter_match(filter, "/home/abc/def/03.txt");
    ck_assert_int_eq(matchno, 4);
    matchno = filter_match(filter, "/home/abc/def/04.txt");
    ck_assert_int_eq(matchno, -1);
    filter_close(filter);
}
END_TEST

static Suite *log_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("FileFilter");
    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_file_filter);
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
