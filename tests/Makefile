CC=gcc

ROOTDIR=../
CFLAGS=-ggdb -O0 -I$(ROOTDIR)/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

LDFLAGS=-lglib-2.0 -lvmi

TESTS=$(shell ls | egrep '^test[0-9]+.c$$' | awk '{split($$0,a,"."); print a[1] }' | sort -n)
#TESTS=test001 test002

all: $(TESTS)

SRCS=$(ROOTDIR)/config.c $(ROOTDIR)/policy.c $(ROOTDIR)/log.c

%:%.c $(SRCS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	for dir in $(TESTS); do\
		rm -rf $$dir; \
	done
