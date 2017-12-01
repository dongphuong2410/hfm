AM_CFLAGS = -I../include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
AM_LDFLAGS =

if DEBUG
AM_CFLAGS += -ggdb -O0
endif

noinst_LIBRARIES = libout.a
libout_a_SOURCES = output_format.c es.c console.c csv.c
