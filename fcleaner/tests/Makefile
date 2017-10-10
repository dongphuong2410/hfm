CC=gcc
CFLAGS=-ggdb -O0
LDFLAGS=

all: test_hashtable

test_hashtable: test_hashtable.c ../hashtable.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

