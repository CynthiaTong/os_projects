#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "common.h"

void error(char *msg) {
  perror(msg);
  exit(1);
}

// both inclusive
int random_in_range(int min, int max) {
  if (max <= min) {
    return min;
  }
  return rand() % (max + 1 - min) + min;
}

sem_t* initialize_sem(char *semname, int init_val) {
  sem_t *sem = (sem_t *) malloc(sizeof(sem_t));
  int init = 0;
  while (!init) {
    sem = sem_open(semname, O_CREAT | O_EXCL, SEM_PERMISSION, init_val);
    if (sem == SEM_FAILED) {
      if (errno == EEXIST) {
        // if existing named sem, unlink and create a new one
        sem_unlink(semname);
      } else {
        error("sem_open");
      }
    } else {
      init = 1;
    }
  }
  return sem;
}

void detach_shm(int *mem) {
  // detach from shm before exit
  if (shmdt((void *)mem) == -1) {
    error("shmdt");
  }
}

void clean_up(sem_t *client_q_sem, sem_t *cashier_array_sem,
sem_t *server_q_sem, int *mem) {
  sem_close(client_q_sem);
  sem_close(cashier_array_sem);
  sem_close(server_q_sem);
  detach_shm(mem);
}

/*
  comparison algo for quick sort a C array
  reference: https://stackoverflow.com/a/1788048
 */
int comp(const void *elem1, const void *elem2) {
    order_item f = *((order_item *)elem1);
    order_item s = *((order_item *)elem2);
    if (f.times_ordered > s.times_ordered) return 1;
    if (f.times_ordered < s.times_ordered) return -1;
    return 0;
}