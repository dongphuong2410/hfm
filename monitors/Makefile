CC=gcc
CFLAGS=-ggdb -O0 -I../include
LDFLAGS=

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

