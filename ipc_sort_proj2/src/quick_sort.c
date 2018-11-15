#include <stdio.h>
#include <stdlib.h>
#include "record.h"
#include "constants.h"
#include "util.h"

int set_pivot(int start, int end) {
  return random_in_range(start, end);  // random pivot
}

/*
  Partition code reference: Robert Sedgewick and Kevin Wayne
  https://algs4.cs.princeton.edu/code/edu/princeton/cs/algs4/Quick.java.html
  The below code has slight modifications from the reference
*/
int partition(record* records[], int start, int end, int sort_attr) {
  int pivot = set_pivot(start, end);
  swap(&records[pivot], &records[end]);

  int i = start - 1;
  int j = end;
  while (1) {
    while (compare_records(records[++i], records[end], sort_attr) < 0) {
      if (i == end) break;
    }
    while (compare_records(records[--j], records[end], sort_attr) > 0) {
      if (j == start) break;
    }
    if (i >= j) break;
    swap(&records[i], &records[j]);
  }
  swap(&records[i], &records[end]);
  return i;
}

void quick_sort(record* records[], int start, int end, int sort_attr) {
  if (start < end) {
    int p = partition(records, start, end, sort_attr);
    quick_sort(records, start, p - 1, sort_attr);
    quick_sort(records, p + 1, end, sort_attr);
  }
}

/*
  inline parameters: filename, start position, end position, sorting_attribute
*/
int main(int argc, char **argv) {
  srand(time(NULL));	// random seed

  if (argc != 5) {
    fprintf(stderr, "Usage: ./QS filename start_pos end_pos sort_attribute\n");
    exit(1);
  }

  char *filename = argv[1];
  int start = atoi(argv[2]);   // atoi is not very safe, use it for now
  int end = atoi(argv[3]);
  int sort_attr = atoi(argv[4]);
  int num_records = count_num_records(start, end);

  record **records = parse_records(filename, start, num_records);

  quick_sort(records, 0, num_records - 1, sort_attr);
  assert_sorted(records, num_records, sort_attr);

  print_records(records, num_records);
  free_records(records, num_records);
  return 0;
}
