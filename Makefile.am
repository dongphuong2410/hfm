AUTOMAKE_OPTIONS = foreign
CFLAGS=-ggdb -O0 -I./include -I./monitors -I./outputs -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include 
LDADD=monitors/libmon.a output/libout.a -lvmi

CFLAGS+= -DLOG_LEVEL=LV_INFO

bin_PROGRAMS=hfm
hfm_SOURCES=main.c log.c config.c policy.c hfm.c xen_helper.c trapmngr.c rekall.c context.c win.c multiwatch.c

