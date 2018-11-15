#ifndef UTIL_H
#define UTIL_H

#include "common.h"
#include <errno.h>
#include <fcntl.h>  // for O_CREAT | O_EXCL
#include <sys/shm.h>

void error(char *msg);
int random_in_range(int min, int max);
sem_t* initialize_sem(char *semname, int init_val);
void detach_shm(int *mem);
void clean_up(sem_t *client_q_sem, sem_t *cashier_array_sem, sem_t *server_q_sem, int *mem);
int comp(const void *elem1, const void *elem2);
#endif