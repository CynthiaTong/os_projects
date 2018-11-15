GCC = gcc
CFLAGS = -g -Wall

TARGETS = mysorter merge_split SH QS BS test_sorted
TIMING_DIR = timing
BINARIES = mysorter a.out *.o merge_split sort QS BS SH test_sorted

all: $(TARGETS)

root.o: root.c constants.h
		$(GCC) $(CFLAGS) root.c -c

mysorter: root.o util.o
		$(GCC) $(CFLAGS) -o mysorter root.o util.o

util.o: util.c util.h
		$(GCC) $(CFLAGS) util.c -c

sort.o: sort.c constants.h util.h
		$(GCC) $(CFLAGS) sort.c -c

sort: sort.o util.o
		$(GCC) $(CFLAGS) -o sort sort.o util.o

merge_split.o: merge_split.c constants.h util.h
		$(GCC) $(CFLAGS) merge_split.c -c

merge_split: merge_split.o util.o sort.o
		$(GCC) $(CFLAGS) -o merge_split merge_split.o util.o sort.o

shell_sort.o: shell_sort.c
		$(GCC) $(CFLAGS) shell_sort.c -c

quick_sort.o: quick_sort.c
		$(GCC) $(CFLAGS) quick_sort.c -c

bubble_sort.o: bubble_sort.c
		$(GCC) $(CFLAGS) bubble_sort.c -c

SH: shell_sort.o util.o
		$(GCC) $(CFLAGS) -o SH shell_sort.o util.o

BS: bubble_sort.o util.o
		$(GCC) $(CFLAGS) -o BS bubble_sort.o util.o

QS: quick_sort.o util.o
		$(GCC) $(CFLAGS) -o QS quick_sort.o util.o

test_sorted: test_sorted.c util.o
		$(GCC) $(CFLAGS) -o test_sorted test_sorted.c util.o

clean:
		rm -rf $(BINARIES) $(TIMING_DIR)
