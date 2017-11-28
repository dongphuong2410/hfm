ROOTDIR=../
AM_CFLAGS = -I$(ROOTDIR)/include -I$(ROOTDIR)/outputs -I$(ROOTDIR)/monitors -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
AM_LDFLAGS=-lglib-2.0 -lvmi -lxenctrl -lxentoollog -lxenlight -ljson-c -lm -lpthread

if DEBUG
AM_CFLAGS += -ggdb -O0
endif

