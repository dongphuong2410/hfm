CC=gcc
CFLAGS=-ggdb -O0
LDFLAGS=

all: test_hashtable test_checkdup test_symlink test_filewatch

test_hashtable: test_hashtable.c ../hashtable.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

test_checkdup: test_checkdup.c ../fileutil.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS) -lssl -lcrypto

test_symlink: test_symlink.c ../fileutil.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS) -lssl -lcrypto

test_filewatch: test_filewatch.c ../filewatch.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS) -lpthread
