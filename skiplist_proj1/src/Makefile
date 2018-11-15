# define c++ compiler
CC = g++
# use c++11 standard
CFLAGS = -std=c++11 -g

TARGETS = main.o io.o myapp.o skiplist.o
BINARIES = myapp a.out main.o io.o myapp.o skiplist.o

all: $(TARGETS)
		$(CC) $(CFLAGS) -o myapp $(TARGETS)

skiplist.o: skiplist.cc skiplist.h
		$(CC) $(CFLAGS) -c skiplist.cc

myapp.o: myapp.cc myapp.h
		$(CC) $(CFLAGS) -c myapp.cc

io.o: io.cc io.h
		$(CC) $(CFLAGS) -c io.cc

main.o: main.cc
		$(CC) $(CFLAGS) -c main.cc

clean:
	 	rm -f $(BINARIES)
