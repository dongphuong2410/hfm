CC=gcc

ROOTDIR=../
CFLAGS=-I$(ROOTDIR)/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

LDFLAGS=-lglib-2.0 -lvmi

TESTS=test001

all: $(TESTS)

%:%.c $(ROOTDIR)/config.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	for dir in $(TESTS); do\
		rm -rf $$dir; \
	done
