DEPDIR := .deps
$(shell mkdir -p $(DEPDIR) > /dev/null)
CC=gcc
CFLAGS=-ggdb -O0 -I../include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS=

OBJECTS=output_format.o es.o console.o csv.o

all: libout.a

libout.a: $(OBJECTS)
	ar rcs libout.a $^

%.o:%.c $(DEPDIR)/%.d
	$(CC) -MM $(CFLAGS) $< > $(DEPDIR)/$*.Td;   \
	sed 's,\($*\)\.o[ :]*,\1.o $*.d : ,g' < $(DEPDIR)/$*.Td > $(DEPDIR)/$*.d; \
	rm -f $(DEPDIR)/$*.Td
	$(CC) -c $< -o $@ $(CFLAGS)
	touch $@

.INTERMEDIATE: $(OBJECTS)

clean:
	rm -f libout.a
	rm -rf .deps

$(DEPDIR)/%.d: ;    
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(OBJECTS:%.o=$(DEPDIR)/%.d))

