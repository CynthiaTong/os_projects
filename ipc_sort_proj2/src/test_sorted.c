#include "util.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: ./test_sorted filename sort_attribute\n");
    exit(1);
  }

  int num_records = num_records_in_file(argv[1]);
  record **records = parse_records(argv[1], 0, num_records);
  int sort_attr = atoi(argv[2]);
  assert_sorted(records, num_records, sort_attr);
  int i;
  for (i = 0; i < num_records; ++i) {
    record *r = records[i];
    printf("%d \t %s \t %s %f\n", r->taxation_number, r->first_name, r->last_name, r->income);
  }
  printf("SORTED!\n");
  return 0;
}
