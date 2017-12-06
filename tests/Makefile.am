TESTS = config_check
check_PROGRAMS = config_check

config_check_SOURCES = config_check.c ../src/config.c
config_check_CFLAGS = -I../src/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include @CHECK_CFLAGS@
config_check_LDADD = @CHECK_LIBS@
