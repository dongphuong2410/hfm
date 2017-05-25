CC=gcc
CFLAGS=-ggdb -O0 -I../include
LDFLAGS=

OBJECTS=output_format.o es.o console.o csv.o

all: $(OBJECTS)
	ar rcs libout.a $^
	rm -f $(OBJECTS)

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f libout.a
	rm -f *.o

