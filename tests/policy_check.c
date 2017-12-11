#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include "policy.h"

START_TEST(test_policy_file_not_exist)
{
    GHashTable *policies = get_policies("notexist.policy");
    ck_assert_ptr_null(policies);
}
END_TEST

START_TEST(test_policy_incorrect)
{
    GHashTable *policies = get_policies("incorrect.policy");
    ck_assert_ptr_nonnull(policies);
    guint policy_cnt = g_hash_table_size(policies);
    ck_assert_int_eq(policy_cnt, 0);
    free_policies(policies);
}
END_TEST

START_TEST(test_policy_correct)
{
    GHashTable *policies = get_policies("correct.policy");
    ck_assert_ptr_nonnull(policies);
    guint policy_cnt = g_hash_table_size(policies);
    ck_assert_int_eq(policy_cnt, 2);
    policy_t *pol;
    int id = 10;
    pol = g_hash_table_lookup(policies, &id);
    ck_assert_ptr_nonnull(pol);
    ck_assert_int_eq(pol->id, id);
    ck_assert_int_eq(pol->type, MON_DELETE);
    ck_assert_int_eq(pol->severity, WARNING);
    ck_assert_str_eq(pol->path, "C:/meo/*");
    ck_assert_int_eq(pol->options, 0);

    id = 30;
    pol = g_hash_table_lookup(policies, &id);
    ck_assert_ptr_nonnull(pol);
    ck_assert_int_eq(pol->id, id);
    ck_assert_int_eq(pol->type, MON_MODIFY_CONTENT);
    ck_assert_int_eq(pol->severity, WARNING);
    ck_assert_str_eq(pol->path, "C:/meo/*");
    ck_assert(pol->options & POLICY_OPTIONS_EXTRACT);

    id = 50;
    pol = g_hash_table_lookup(policies, &id);
    ck_assert_ptr_null(pol);

    free_policies(policies);
}
END_TEST

static Suite *log_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Policy");
    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_policy_file_not_exist);
    tcase_add_test(tc_core, test_policy_incorrect);
    tcase_add_test(tc_core, test_policy_correct);
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
