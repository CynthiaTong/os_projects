#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include "util.h"
#include "constants.h"
#include "sort.h"

/*
  send result back to parent via another pipe, or if identifier is 0 (first merger),
  print result to stdout / output file
*/
void merge_pipes(int cur_fd[], int fd1[], int fd2[], int sort_attr) {
  // close the write end of children pipes
  close(fd1[WRITE]);
  close(fd2[WRITE]);

  record r1, r2;
  if (read_record(fd1[READ], &r1) != DATA_RECORD_LENGTH) {
      perror("Merger: read failed");
      exit(1);
  }
  if (read_record(fd2[READ], &r2) != DATA_RECORD_LENGTH) {
      perror("Merger: read failed");
      exit(1);
  }

  int read_res;
  int r1_finished = 0, r2_finished = 0;
  while (1) {
    if (compare_records(&r1, &r2, sort_attr) < 0) {
      // send result back to parent via the given pipe
      if (write_record(cur_fd[WRITE], &r1) != DATA_RECORD_LENGTH) {
        perror("Merger: write failed");
        exit(1);
      }
      read_res = read_record(fd1[READ], &r1);
      if (read_res == 0) {
        r1_finished = 1;
        break;
      } else if (read_res != DATA_RECORD_LENGTH) {
        perror("Merger: read failed");
        exit(1);
      }
    } else {
      if (write_record(cur_fd[WRITE], &r2) != DATA_RECORD_LENGTH) {
        perror("Merger: write failed");
        exit(1);
      }
      read_res = read_record(fd2[READ], &r2);
      if (read_res == 0) {
        r2_finished = 1;
        break;
      } else if (read_res != DATA_RECORD_LENGTH) {
        perror("Merger: read failed");
        exit(1);
      }
    }
  }
  // write the last valid record instance from the ablove merge procedure and the rest leftovers
  while (!r1_finished) {
    if (write_record(cur_fd[WRITE], &r1) != DATA_RECORD_LENGTH) {
      perror("Merger: write failed");
      exit(1);
    }
    r1_finished = read_record(fd1[READ], &r1) > 0 ? 0 : 1;
  }
  while (!r2_finished) {
    if (write_record(cur_fd[WRITE], &r2) != DATA_RECORD_LENGTH) {
      perror("Merger: write failed");
      exit(1);
    }
    r2_finished = read_record(fd2[READ], &r2) > 0 ? 0 : 1;
  }

  close(fd1[READ]);
  close(fd2[READ]);
  close(cur_fd[WRITE]);
}

void split_range(int *c1_range, int *c2_range, int start, int end, int random, int depth) {
  int num_leaves_in_subtree = num_leaf_sorters(depth - 1);
  if (count_num_records(start, end) < 2 * num_leaves_in_subtree) {
    fprintf(stderr, "Invalid range for splitting: %d, %d\n", start, end);
    exit(1);
  }
  int mid;
  // if random flag is set and the given range does not have to be split evenly among the two children
  if (random && (count_num_records(start, end) != 2 * num_leaves_in_subtree)) {
      mid = random_in_range(start + num_leaves_in_subtree, end - num_leaves_in_subtree);
  } else {
    mid = (start + end) / 2;
  }
  c1_range[0] = start;
  c1_range[1] = mid;
  c2_range[0] = mid + 1;
  c2_range[1] = end;
}

int main(int argc, char **argv) {
  srand(time(NULL));	// random seed

  if (argc != 6) {
    printf("Usage: ./merge_sort filename tree_depth sort_attribute num_records random_split\n");
    exit(1);
  }
  int root_pid = getppid();

  // parse arguments
  char *filename = argv[1];
  int depth = atoi(argv[2]);  // depth_str
  int sorter_index_offset = num_leaf_sorters(depth) - 1; // -1 because there is one less internal node than leaf nodes

  int sort_attribute = atoi(argv[3]); // which attribute to sort on

  int current_range[2];
  current_range[0] = 0;
  current_range[1] = atoi(argv[4]) - 1; // num_records - 1
  int random_split = atoi(argv[5]); // whether split range randomly

  int identifier = 0; // identifies the current (first ms) process
  int current_fd[2];  // fd of the pipe from parent of this current process
  current_fd[1] = STDOUT_FILENO;  // standard out or output file if specified

  int c1_pid, c2_pid;
  int c1_range[2], c2_range[2];
  int fd1[2], fd2[2];   // pipe fds for each child

  while (1) {
    // new parent: split ranges and create pipes!
    split_range(c1_range, c2_range, current_range[0], current_range[1], random_split, depth);
    create_pipe(fd1);
    create_pipe(fd2);

    if ((c1_pid = create_child()) && (c2_pid = create_child())) {   // if parent process
      clock_t tic = clock();
      // wait for children, and merge result from the two pipes
      merge_pipes(current_fd, fd1, fd2, sort_attribute);
      // wait for the two children to avoid zombie state
      while (waitpid(c1_pid, NULL, WNOHANG) == 0 || waitpid(c2_pid, NULL, WNOHANG) == 0) {}
      clock_t toc = clock();
      double duration_ms = 1000.0 * (toc - tic) / CLOCKS_PER_SEC;
      // write runtime data to a file in the timing dir
      char timing_filename[sizeof(TIMING_DIR) + 6 + sizeof(int)];
      sprintf(timing_filename, "%smerger%d", TIMING_DIR, identifier);
      write_timing_data(timing_filename, current_range[0], current_range[1], duration_ms);
      //fprintf(stderr, "merger %d\t %d , %d \t %f\n", identifier, current_range[0], current_range[1], duration_ms);
      break;

    } else if (c1_pid == 0) {   // c1
      // update c1 identifiers, ranges and close fds accordingly
      identifier = c1_identifier(identifier);
      current_range[0] = c1_range[0];
      current_range[1] = c1_range[1];
      current_fd[0] = fd1[0];
      current_fd[1] = fd1[1];

      close(fd1[READ]);
      close(fd2[READ]);
      close(fd2[WRITE]);

      --depth;
      // sorter if depth is 0, internal node otherwise
      if (depth == 0) {
        sorter_work(identifier, filename, current_range, current_fd, sort_attribute,
            root_pid, sorter_index_offset);
        break;
      }

    } else {    // c2
      // update c2 identifiers, ranges and close fds accordingly
      identifier = c2_identifier(identifier);
      current_range[0] = c2_range[0];
      current_range[1] = c2_range[1];
      current_fd[0] = fd2[0];
      current_fd[1] = fd2[1];

      close(fd2[READ]);
      close(fd1[READ]);
      close(fd1[WRITE]);

      --depth;
      // sorter if depth is 0, internal node otherwise
      if (depth == 0) {
        sorter_work(identifier, filename, current_range, current_fd, sort_attribute,
            root_pid, sorter_index_offset);
        break;
      }
    }
  }

  return 0;
}