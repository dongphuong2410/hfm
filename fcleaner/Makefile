CC=gcc
CFLAGS=-ggdb -O0
LDFLAGS=

all: fcleaner

fcleaner: main.c filewatch.c hashtable.c fileutil.c
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)
