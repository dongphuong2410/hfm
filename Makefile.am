AUTOMAKE_OPTIONS = foreign
AM_CFLAGS= -I./include -I./monitors -I./outputs -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include 
LDADD=monitors/libmon.a outputs/libout.a -lvmi

if LOG_LEVEL_SET
AM_CFLAGS+= -DLOG_LEVEL=$(LOG)
else    
AM_CFLAGS+= -DLOG_LEVEL=LV_INFO
endif

if DEBUG
AM_CFLAGS+= -ggdb -O0
endif

SUBDIRS=monitors outputs
bin_PROGRAMS=hfm
hfm_SOURCES=main.c log.c config.c policy.c hfm.c xen_helper.c trapmngr.c rekall.c context.c win.c multiwatch.c

