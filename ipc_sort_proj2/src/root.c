#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <wait.h>
#include <time.h>

#include "constants.h"
#include "util.h"

extern char *optarg;
extern int optind, optopt;

int sh_signal_count = 0;
int qs_signal_count = 0;
int bs_signal_count = 0;

void print_usage() {
  printf("Usage: ./mysorter -d TreeDepth -f RecsFile -a AttrNum -o OutFile -r\n");
  exit(1);
}

void check_required_args(int depth, int sort_attr, char *in) {
  if (depth < MIN_DEPTH || depth > MAX_DEPTH) {
    printf("Input Error: required -d argument range is [1, 6]\n");
    print_usage();
  }
  if (sort_attr < SORT_ON_TAX || sort_attr > SORT_ON_INCOME) {
    printf("Input Error: required -a argument range is [0, 3]\n");
    print_usage();
  }
  if (in == NULL) {
    printf("Input Error: input file is required\n");
    print_usage();
  }
}

void sigusr1_hanlder() {
  signal(SIGUSR1, sigusr1_hanlder);
  ++sh_signal_count;
}

void sigusr2_hanlder() {
  signal(SIGUSR2, sigusr2_hanlder);
  ++qs_signal_count;
}

void sigalrm_hanlder() {
  signal(SIGALRM, sigalrm_hanlder);
  ++bs_signal_count;
}

int main(int argc, char **argv) {

  int depth = 0; // user specified sorting tree depth
  int random_split = 0; // whether split range randomly
  int sort_attr = -1; // which attribute to sort on
  char *input_file = NULL;
  char *output_file = NULL;

  // getopt to parse command line arguments
  int opt;
  while ((opt = getopt(argc, argv, ":d:f:o:a:r")) != -1) {
    switch(opt) {
    case 'd':
        depth = atoi(optarg);
        //printf("depth: %d\n", depth);
        break;
    case 'f':
        input_file = optarg;
        //printf("input_file is %s\n", input_file);
        break;
    case 'o':
        output_file = optarg;
        //printf("output_file is %s\n", output_file);
        break;
    case 'a':
        sort_attr = atoi(optarg);
        //printf("sort_attr is %d\n", sort_attr);
        break;
    case 'r':
      random_split = 1;
      break;
    case ':':
        printf("Input Error: -%c without argument\n", optopt);
        print_usage();
    case '?':
        printf("Input Error: unknown option %c\n", optopt);
        print_usage();
    }
  }
  // detect unknown arguments
  int i;
  for (i = optind; i < argc; i++) {
    printf("Input Error: unknown argument %s\n", argv[i]);
    print_usage();
  }
  check_required_args(depth, sort_attr, input_file);

  int num_records = num_records_in_file(input_file);
  int num_sorters = num_leaf_sorters(depth);
  if (num_records == 0) {
    printf("Input file %s is empty\n", input_file);
    exit(1);
  } else if (num_sorters > num_records) {
    printf("Usage Error: number of sorters (%d) cannot be more than number of records (%d) \n",
        num_sorters, num_records);
    exit(1);
  }

  setup_timing_dir();
  // setup signals
  signal(SIGUSR1, sigusr1_hanlder);
  signal(SIGUSR2, sigusr2_hanlder);
  signal(SIGALRM, sigalrm_hanlder);

  int ms_pid = create_child();
  // start keeping track of time
  if (ms_pid > 0) {   // root
    // wait for ms
    clock_t tic = clock();
    while (waitpid(ms_pid, NULL, WNOHANG) == 0) {}
    clock_t toc = clock();
    double duration_ms = 1000.0 * (toc - tic) / CLOCKS_PER_SEC;
    // print signal count data, compare to expectation
    int q = num_sorters / NUM_OF_SORTS;
    int r = num_sorters % NUM_OF_SORTS;
    int sh_signal_expected = q + (r >= 1 ? 1 : 0);
    int qs_signal_expected = q + (r == 2 ? 1 : 0);
    int bs_signal_expected = q;

    // system call to cat all files in the timing dir
    system("exec cat timing/*");
    printf("Shell Sort (SIGUSR1): received/expected = %d / %d\n", sh_signal_count, sh_signal_expected);
    printf("Quick Sort (SIGUSR2): received/expected = %d / %d\n", qs_signal_count, qs_signal_expected);
    printf("Bubble Sort (SIGALRM): received/expected = %d / %d\n", bs_signal_count, bs_signal_expected);
    printf("Sorting turnaround time: %f ms\n", duration_ms);

  } else {   // first merger/splitter
    // if output file specified, redirect standard output to a file using freopen
    if (output_file != NULL) {
      FILE *out = freopen(output_file, "wb", stdout);
      if (out == NULL) {
        printf("failed to open binary output file\n");
		    exit(1);
      }
    }
    // exec m/s code, pass in arguments:
    // root_pid, range, input file, sort attr, random
    char depth_str[sizeof(int)];
    char sort_attr_str[sizeof(int)];
    char random_split_str[sizeof(int)];
    char num_records_str[sizeof(int)];
    sprintf(depth_str, "%d", depth);
    sprintf(sort_attr_str, "%d", sort_attr);
    sprintf(random_split_str, "%d", random_split);
    sprintf(num_records_str, "%d", num_records);

    execlp(MERGE_SPLIT, MERGE_SPLIT, input_file, depth_str, sort_attr_str,
        num_records_str, random_split_str, NULL);
    perror("execlp failed");
  }
  return 0;
}
