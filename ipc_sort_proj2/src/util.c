#include "util.h"

// both inclusive
int random_in_range(int min, int max) {
	return rand() % (max + 1 - min) + min;
}

/*
  Suppose tree depth is k > 0, number of leaves is: 2^k
*/
int num_leaf_sorters(int depth) {
  return (1 << depth);
}

int c1_identifier(int parent_id) {
  return 2 * parent_id + 1;
}

int c2_identifier(int parent_id) {
  return 2 * parent_id + 2;
}

char* which_sort(int identifier) {
  int mod = identifier % NUM_OF_SORTS;
  if (mod == USE_SHELL_SORT) {
    return SH;
  } else if (mod == USE_QUICK_SORT) {
    return QS;
  } else {  // USE_BUBBLE_SORT
    return BS;
  }
}

void swap(record **r1, record **r2) {
  record *tmp = *r1;  // have to save *r1 in tmp, note: cannot use record ** !
  *r1 = *r2;
  *r2 = tmp;
}

int create_child() {
  int pid = fork();
  if (pid == -1) {
    perror("fork failed");
    exit(1);
  }
  return pid;
}

void create_pipe(int fd[]) {
  if (pipe(fd) == -1) {
    perror("pipe failed");
    exit(1);
  }
}

// both inclusive
int count_num_records(int start, int end) {
  return end + 1 - start;
}

int num_records_in_file(char *filename) {
  FILE *fp;
	fp = fopen(filename,"rb");
	if (fp == NULL) {
		fprintf(stderr, "failed to open binary file\n");
		exit(1);
	}
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp); // file size in bytes
  fclose(fp);
  return size / DATA_RECORD_LENGTH;
}

record** parse_records(char *filename, int start, int num_records) {
  FILE *fp;
	fp = fopen(filename,"rb");
	if (fp == NULL) {
		fprintf(stderr, "failed to open binary file\n");
		exit(1);
	}

	// place fp to start pos and set number of records to sort
	fseek(fp, start * DATA_RECORD_LENGTH, SEEK_SET);
  record **records = malloc(sizeof(record*) * num_records); // array of pointers to record structs
  int i;
	for (i = 0; i < num_records; ++i) {
    records[i] = (record*) malloc(sizeof(record));
    fread(records[i], DATA_RECORD_LENGTH, 1, fp);
	}
	fclose(fp);
  return records;
}

/*
  Normally return total_bytes_read should equal to DATA_RECORD_LENGTH
*/
int read_record(int read_fd, record *r) {
  int total_bytes_read = 0;
  int bytes_read = 0;
  while (1) {
    // r + bytes_read: read from where we left off if previous reads do not return enough bytes
    bytes_read = read(read_fd, r + bytes_read, sizeof(*r) - bytes_read);
    if (bytes_read < 0) {
      return -1;
    } else {
      if (bytes_read == 0) {   // end of record item
        return total_bytes_read;
      }
      total_bytes_read += bytes_read;
    }
  }
}

int write_record(int write_fd, record *r) {
  int total_bytes_written = 0;
  int bytes_written = 0;
  while (1) {
    // r + bytes_written: read from where we left off if previous reads do not return enough bytes
    bytes_written = write(write_fd, r + bytes_written, sizeof(*r) - bytes_written);
    if (bytes_written < 0) {
      return -1;
    } else {
      if (bytes_written == 0) {   // end of record item
        return total_bytes_written;
      }
      total_bytes_written += bytes_written;
    }
  }
}

/*
  print records to stdout
*/
void print_records(record **records, int num_records) {
  int i;
  for (i = 0; i < num_records; ++i) {
    if (write_record(STDOUT_FILENO, records[i]) < 0) {
      perror("write failed");
      exit(1);
    }
  }
}

void free_records(record **records, int num_records) {
  int i;
  for (i = 0; i < num_records; ++i) {
    free(records[i]);
  }
  free(records);
}

int compare_records(record *r1, record *r2, int sort_attribute) {
  switch (sort_attribute) {
    case SORT_ON_TAX:
      return r1->taxation_number - r2->taxation_number;
    case SORT_ON_FIRST:
      return strcmp(r1->first_name, r2->first_name);  // negative if r1->first < r2->first
    case SORT_ON_LAST:
      return strcmp(r1->last_name, r2->last_name);
    case SORT_ON_INCOME:
      return r1->income - r2->income;
    default:
      return -1;
  }
}

/*
  assuming no duplicate data entry
*/
void assert_sorted(record* recs[], int size, int sort_attribute) {
  int i;
  for (i = 1; i < size; ++i) {
    assert(compare_records(recs[i-1], recs[i], sort_attribute) <= 0);
  }
}

/*
  Below code for removing files inside a dir is authored by: Demitri,
  posted on stackoverflow: https://stackoverflow.com/a/11007597
*/
void setup_timing_dir() {
  // set up timing data directory
  if (mkdir(TIMING_DIR, S_IRWXU) == -1) { // S_IRWXU: user Read + Write + Execute
    // if timing dir already exists
    if (errno == EEXIST) {
      DIR *folder = opendir(TIMING_DIR);
      struct dirent *next_file;
      char filepath[256];
      while ((next_file = readdir(folder)) != NULL) {
        sprintf(filepath, "%s/%s", TIMING_DIR, next_file->d_name);
        remove(filepath);
      }
      closedir(folder);
    } else {
      perror("failed to create timing directory\n");
      exit(1);
    }
  }
}

void write_timing_data(char *name, int start_index, int end_index, double duration) {
  FILE *fp = fopen(name, "w+");
  if (fp == NULL) {
    fprintf(stderr, "failed to open timing file\n");
    return;
  }
  fprintf(fp, "%s \t record range: %d,%d \t duration: %f ms\n", name + sizeof(TIMING_DIR) - 1, // omit dir name
      start_index, end_index, duration);
}