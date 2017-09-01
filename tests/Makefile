CC=gcc

ROOTDIR=../
CFLAGS=-ggdb -O0 -I$(ROOTDIR)/include -I$(ROOTDIR)/outputs -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

LDFLAGS=-lglib-2.0 -lvmi -lxenctrl -lxentoollog -lxenlight -ljson-c -lm

TESTS=$(shell ls | egrep '^test[0-9]+.c$$' | awk '{split($$0,a,"."); print a[1] }' | sort -n)

all: $(TESTS)

SRCS=$(ROOTDIR)/hfm.c $(ROOTDIR)/xen_helper.c $(ROOTDIR)/config.c $(ROOTDIR)/policy.c $(ROOTDIR)/log.c $(ROOTDIR)/file_filter.c $(ROOTDIR)/fnmatch.c  $(ROOTDIR)/trapmngr.c $(ROOTDIR)/outputs/libout.a $(ROOTDIR)/rekall.c $(ROOTDIR)/context.c $(ROOTDIR)/win.c $(ROOTDIR)/util.c

%:%.c $(SRCS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	for dir in $(TESTS); do\
		rm -rf $$dir; \
	done
