AM_CFLAGS=-ggdb -O0
AM_LDFLAGS=

bin_PROGRAMS = test_hashtable test_checkdup test_symlink test_filewatch

test_hashtable_LDADD = -lssl -lcrypto
test_symlink_LDADD = -lssl -lcrypto
test_filewatch_LDADD = -lpthread

test_hashtable_SOURCES = test_hashtable.c ../hashtable.c
test_checkdup_SOURCES = test_checkdup.c ../fileutil.c
test_symlink_SOURCES = test_symlink.c ../fileutil.c
test_filewatch_SOURCES = test_filewatch.c ../filewatch.c
