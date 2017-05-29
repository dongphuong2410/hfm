CC=gcc
CFLAGS=-ggdb -O0 -I./include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS=-lvmi -lglib-2.0

SUBDIRS = monitors outputs

SRC= main.c log.c config.c policy.c hfm.c vmi_helper.c file_filter.c

CFLAGS+= -DCURRENT_LEVEL=LV_WARN
.PHONY : all $(SUBDIRS) clean

all: hfm $(SUBDIRS)

hfm: $(SRC)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(SUBDIRS):
	$(MAKE) -C $@ clean all


clean:
	rm -rf hfm
	for dir in $(SUBDIRS); do\
		$(MAKE) -C $$dir -f Makefile $@; \
	done


