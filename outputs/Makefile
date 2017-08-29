CC=gcc
CFLAGS=-ggdb -O0 -I../include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

LDFLAGS=

OBJECTS=output_format.o es.o console.o csv.o

all: libout.a

libout.a: $(OBJECTS)
	ar rcs libout.a $^

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f libout.a
	rm -f *.o

