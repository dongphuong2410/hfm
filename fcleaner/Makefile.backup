CC=gcc
CFLAGS=-ggdb -O0
LDFLAGS=-lssl -lcrypto -lpthread

all: fcleaner

fcleaner: main.c filewatch.c hashtable.c fileutil.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)
