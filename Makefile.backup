DEPDIR=.deps
$(shell mkdir -p $(DEPDIR) > /dev/null)
CC=gcc
CFLAGS=-ggdb -O0 -I./include -I./monitors -I./outputs -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include 
LDFLAGS=-lvmi -lglib-2.0 -lxenctrl -lxentoollog -lxenlight -ljson-c -lm -lpthread
DEPFLAGS=-MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

POSTCOMPILE=@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

STATICLIB=./monitors/libmon.a ./outputs/libout.a

SUBDIRS = monitors outputs tests

SRC= main.c log.c config.c policy.c hfm.c xen_helper.c trapmngr.c rekall.c context.c win.c multiwatch.c

CFLAGS+= -DLOG_LEVEL=LV_INFO
.PHONY : all $(SUBDIRS) clean

all: $(SUBDIRS) hfm

hfm: $(SRC:%.c=%.o) $(STATICLIB)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(SUBDIRS):
	$(MAKE) -C $@ all

%.o : %.c
%.o : %.c $(DEPDIR)/%.d
	$(CC) $(CFLAGS) $(DEPFLAGS) $< -c
	$(POSTCOMPILE)

.INTERMEDIATE: $(SRC:%.c=%.o)

clean:
	rm -rf hfm
	for dir in $(SUBDIRS); do\
		$(MAKE) -C $$dir -f Makefile $@; \
	done

TESTDIR=tests
include UnitTest.mk

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(SRC:%.c=$(DEPDIR)/%.d))
