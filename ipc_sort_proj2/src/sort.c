#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "util.h"
#include "constants.h"

void sorter_work(int id, char *filename, int range[], int fd[], int sort_attr, int root_pid, int index_offset) {
  // create pipe to receive data from sort executable
  int sort_fd[2];
  create_pipe(sort_fd);

  record buffer;
  clock_t tic = clock();
  // fork and execute merges
  int pid = create_child();
  if (pid) {
    close(sort_fd[WRITE]);
    // receive data from sort exec
    int read_res;
    while (1) {
      read_res = read_record(sort_fd[READ], &buffer);
      if (read_res == 0) {
        break;
      } else if (read_res != DATA_RECORD_LENGTH) {
        perror("Sorter: read failed");
        exit(1);
      }
      // write back to parent
      if (write_record(fd[WRITE], &buffer) < 0) {
        perror("Sorter: write failed");
        exit(1);
      }
    }
    close(sort_fd[READ]);
    close(fd[WRITE]);
    // blocking wait on the child (sort exec) process
    while (waitpid(pid, NULL, WNOHANG) == 0) {}
    clock_t toc = clock();
    double duration_ms = 1000.0 * (toc - tic) / CLOCKS_PER_SEC;
    // print out runtime data
    char timing_filename[sizeof(TIMING_DIR) + 6 + sizeof(int)];
    sprintf(timing_filename, "%ssorter%d", TIMING_DIR, id - index_offset);
    write_timing_data(timing_filename, range[0], range[1], duration_ms);
    //fprintf(stderr, "sorter %d\t %d , %d\t %f\n", id - index_offset, range[0], range[1], duration_ms);

    // signal the root about sort completion
    int sort_num = (id - index_offset) % NUM_OF_SORTS;
    if (sort_num == USE_SHELL_SORT) {
      kill(root_pid, SIGUSR1);
    } else if (sort_num == USE_QUICK_SORT) {
      kill(root_pid, SIGUSR2);
    } else if (sort_num == USE_BUBBLE_SORT) {
      kill(root_pid, SIGALRM);
    }

  } else {
    // choose sorting based on id
    char *sort_exec = which_sort(id - index_offset);

    // convert parameters to char array
    char start_str[sizeof(int)], end_str[sizeof(int)], sort_attr_str[sizeof(int)];
    sprintf(start_str, "%d", range[0]);
    sprintf(end_str, "%d", range[1]);
    sprintf(sort_attr_str, "%d", sort_attr);

    // dup and execl, writing sorted data to stdout
    close(sort_fd[READ]);
    dup2(sort_fd[WRITE], STDOUT_FILENO);
    close(sort_fd[WRITE]);

    // execlp is used to make argv[0] the executable name
    execlp(sort_exec, sort_exec, filename, start_str, end_str, sort_attr_str, NULL);
    perror("execlp failed");
  }
}
