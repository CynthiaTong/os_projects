
GCC = gcc
CFLAGS = -Wall -std=gnu99

BIN_TARGET = coordinator cashier server client *.o

all: $(BIN_TARGET)

util.o: util.c
	$(GCC) $(CFLAGS) -c util.c

coordinator.o: coordinator.c common.h util.h
	$(GCC) $(CFLAGS) -c coordinator.c

coordinator: coordinator.o util.o
	$(GCC) $(CFLAGS) -o coordinator coordinator.o util.o -lpthread

cashier.o: cashier.c  common.h util.h
	$(GCC) $(CFLAGS) -c cashier.c

cashier: cashier.o util.o
	$(GCC) $(CFLAGS) -o cashier cashier.o util.o -lpthread

client.o: client.c common.h util.h
	$(GCC) $(CFLAGS) -c client.c

client: client.o util.o
	$(GCC) $(CFLAGS) -o client client.o util.o -lpthread

server.o: server.c common.h util.h
	$(GCC) $(CFLAGS) -c server.c

server: server.o util.o
	$(GCC) $(CFLAGS) -o server server.o util.o -lpthread

clean:
	rm -rf $(BIN_TARGET)