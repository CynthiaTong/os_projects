#include <stdio.h>
#include <stdlib.h>
#include "util.h"

/*
  Shell sort code reference: Robert Sedgewick and Kevin Wayne
  https://algs4.cs.princeton.edu/code/edu/princeton/cs/algs4/Shell.java.html
*/

void shell_sort(record **records, int num_records, int sort_attr) {
  // 3x+1 increment sequence:  1, 4, 13, 40, 121, 364, 1093, ...
  int h = 1;
  while (h < num_records / 3) {
    h = 3 * h + 1;
  }

  while (h >= 1) {
      // h-sort the array
      int i, j;
      for (i = h; i < num_records; i++) {
          for (j = i; j >= h && compare_records(records[j], records[j-h], sort_attr) < 0; j -= h) {
              swap(&records[j], &records[j-h]);
          }
      }
      h /= 3;
  }
}

int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr, "Usage: ./SH filename start_pos end_pos sort_attribute\n");
    exit(1);
  }

  char *filename = argv[1];
  int start = atoi(argv[2]);   // atoi is not very safe, use it for now
  int end = atoi(argv[3]);
  int sort_attr = atoi(argv[4]);
  int num_records = count_num_records(start, end);

  record **records = parse_records(filename, start, num_records);
  shell_sort(records, num_records, sort_attr);
  assert_sorted(records, num_records, sort_attr);

  print_records(records, num_records);
  free_records(records, num_records);
  return 0;
}
