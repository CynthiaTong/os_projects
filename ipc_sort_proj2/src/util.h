#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h> // for checking if a directory exists
#include <errno.h>
#include <dirent.h>

#include "record.h"
#include "constants.h"

int random_in_range(int min, int max);
int num_leaf_sorters(int depth);

int create_child();
void create_pipe(int fd[]);
int c1_identifier(int parent_id);
int c2_identifier(int parent_id);

char* which_sort(int identifier);
void swap(record **r1, record **r2);

int count_num_records(int start, int end);
int num_records_in_file(char *filename);

record** parse_records(char *filename, int start, int num_records);
void free_records(record **records, int num_records);
int compare_records(record *r1, record *r2, int sort_attribute);
void assert_sorted(record* recs[], int size, int sort_attribute);

int read_record(int read_fd, record *r);
int write_record(int write_fd, record *r);
void print_records(record **records, int num_records);

void setup_timing_dir();
void write_timing_data(char *name, int start_index, int end_index, double duration);

#endif
