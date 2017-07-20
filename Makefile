CC=gcc
CFLAGS=-ggdb -O0 -I./include -I./monitors -I./outputs -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include 
LDFLAGS=-lvmi -lglib-2.0 -lxenctrl -lxentoollog -lxenlight -ljson-c

STATICLIB=./monitors/libmon.a ./outputs/libout.a

SUBDIRS = monitors outputs tests

SRC= main.c log.c config.c policy.c hfm.c xen_helper.c file_filter.c fnmatch.c trapmngr.c rekall.c context.c

CFLAGS+= -DLOG_LEVEL=LV_WARN
.PHONY : all $(SUBDIRS) clean

all: $(SUBDIRS) hfm

hfm: $(SRC) $(STATICLIB)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(SUBDIRS):
	$(MAKE) -C $@ clean all


clean:
	rm -rf hfm
	for dir in $(SUBDIRS); do\
		$(MAKE) -C $$dir -f Makefile $@; \
	done

TESTDIR=tests
include UnitTest.mk
