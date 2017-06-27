CC=gcc
CFLAGS=-ggdb -O0 -I../include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS=glib-2.0

#OBJECTS=monitor.o file_created.o file_deleted.o hidden_changed.o permission_changed.o owner_changed.o
OBJECTS=monitor.o file_created.o file_deleted.o hidden_changed.o permission_changed.o owner_changed.o

all: $(OBJECTS)
	ar rcs libmon.a $^
	rm -f $(OBJECTS)

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f libmon.a
	rm -f *.o

