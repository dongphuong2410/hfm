AM_CFLAGS=
AM_LDFLAGS=-lssl -lcrypto -lpthread

if DEBUG
AM_CFLAGS += -ggdb -O0
endif

bin_PROGRAMS = fcleaner
fcleaner_SOURCES =  main.c filewatch.c hashtable.c fileutil.c
