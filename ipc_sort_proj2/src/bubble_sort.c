#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void bubble_sort(record **records, int num_records, int sort_attr) {
  int swapped = 1;
  int n = num_records;
  int i;
  while (swapped) {
    swapped = 0;
    for (i = 1; i < n; ++i) {
      if (compare_records(records[i-1], records[i], sort_attr) > 0) {
        swap(&records[i-1], &records[i]);
        swapped = 1;
      }
    }
    --n;
  }
}

int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr, "Usage: ./BS filename start_pos end_pos sort_attribute\n");
    exit(1);
  }

  char *filename = argv[1];
  int start = atoi(argv[2]);   // atoi is not very safe, use it for now
  int end = atoi(argv[3]);
  int sort_attr = atoi(argv[4]);
  int num_records = count_num_records(start, end);
  // printf("%s \t %d %d %d\n", filename, start, end, sort_attr);

  record **records = parse_records(filename, start, num_records);
  bubble_sort(records, num_records, sort_attr);
  assert_sorted(records, num_records, sort_attr);

  print_records(records, num_records);
  free_records(records, num_records);
  return 0;
}
