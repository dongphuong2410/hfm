DEPDIR=.deps
$(shell mkdir -p $(DEPDIR) > /dev/null)
CC=gcc
CFLAGS=-ggdb -O0 -I../include -I../outputs -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS=glib-2.0

OBJECTS=monitor.o file_created.o file_deleted.o file_modified.o owner_changed.o attr_changed.o

all: libmon.a

libmon.a: $(OBJECTS)
	ar rcs libmon.a $^

%.o:%.c $(DEPDIR)/%.d
	$(CC) -MM $(CFLAGS) $< > $(DEPDIR)/$*.Td;   \
	sed 's,\($*\)\.o[ :]*,\1.o $*.d : ,g' < $(DEPDIR)/$*.Td > $(DEPDIR)/$*.d; \
	rm -f $(DEPDIR)/$*.Td  
	$(CC) -c $< -o $@ $(CFLAGS)
	touch $@

.INTERMEDIATE: $(OBJECTS) 

clean:
	rm -f libmon.a
	rm -rf .deps

$(DEPDIR)/%.d: ;    
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(OBJECTS:%.o=$(DEPDIR)/%.d))

