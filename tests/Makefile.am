TESTS = config_check log_check policy_check
check_PROGRAMS = config_check log_check policy_check

config_check_SOURCES = config_check.c ../src/config.c
config_check_CFLAGS = -I../src/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include @CHECK_CFLAGS@
config_check_LDADD = @CHECK_LIBS@

log_check_SOURCES = log_check.c ../src/log.c
log_check_CFLAGS = @CHECK_CFLAGS@ -I../src/include
log_check_LDADD = @CHECK_LIBS@

policy_check_SOURCES = policy_check.c ../src/policy.c
policy_check_CFLAGS = @CHECK_CFLAGS@ -I../src/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
policy_check_LDADD = @CHECK_LIBS@
