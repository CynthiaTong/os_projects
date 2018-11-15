#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>
#include <time.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "common.h"
#include "util.h"

int *mem;
int itemid = -1;
int max_eat_time = -1;
int shmid = -1;
int num_cashiers = -1;

sem_t *client_q_sem;
sem_t *cashier_array_sem;
sem_t *server_q_sem;

cashier_client_info *cinfo; // cashier_client_info struct for a particular cashier
server_client_info *sinfo; // // server_client_info struct for passing data to the server
int *shm_offset;
client_db_entry *db_offset;
int db_index;

void join_client_queue() {
  sem_wait(client_q_sem);
  int num_clients = mem[0];
  int total_num_clients = mem[1];
  if (num_clients >= MAX_NUM_CLIENTS_INLINE) {
    printf("client queue is full. Exit.\n");
    sem_post(client_q_sem);
    clean_up(client_q_sem, cashier_array_sem, server_q_sem, mem);
    exit(0);
  } else if (total_num_clients > MAX_NUM_CLIENTS_TOTAL) {
    printf("Exceeded maximum total number of clients.\n");
    sem_post(client_q_sem);
    clean_up(client_q_sem, cashier_array_sem, server_q_sem, mem);
    exit(0);
  }
  ++mem[0]; // increment num of clients in queue
  db_index = mem[1]++; // increment total num clients ever in diner
  sem_post(client_q_sem);
}

int take_a_cashier() {
  int found = -1;
  while (found == -1) {
    sem_wait(cashier_array_sem);
    for (int i = 0; i < num_cashiers; ++i) {
      if (mem[i + SHM_INT_OFFSET] == 1) {  // found available cashier and take the spot
        mem[i + SHM_INT_OFFSET] = 0;
        found = i;
        break;
      }
    }
    sem_post(cashier_array_sem);
  }
  return found;
}

void write_total_time_to_db(double duration) {
  client_db_entry *db_entry = db_offset + db_index;
  db_entry->client_total_time = duration;
}

int print_usage() {
  printf("Usage: ./client -i itemId -e eatTime -m shmid -n numCashiers\n");
  exit(1);
}

void verify_inputs() {
  if (itemid != -1 && (itemid < 1 || itemid > 20)) {
    printf("Input Error: client item id must be in range [1, 20]\n");
    print_usage();
  } else if (itemid == -1 || max_eat_time == -1 || shmid == -1 || num_cashiers == -1) {
    print_usage();
  }
}

int main(int argc, char **argv) {
  srand(time(NULL));    // random seed
    // take and verify user input
  int opt;
  while ((opt = getopt(argc, argv, ":i:e:m:n:")) != -1) {
    switch(opt) {
      case 'i':
        itemid = atoi(optarg);
        //printf("itemid: %d\n", itemid);
        break;
      case 'e':
        max_eat_time = atoi(optarg);
        //printf("max_eat_time %d\n", max_eat_time);
        break;
      case 'm':
        shmid = atoi(optarg);
        //printf("shmid is %d\n", shmid);
        break;
      case 'n':
        num_cashiers = atoi(optarg);
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
  for (int i = optind; i < argc; ++i) {
    printf("Input Error: unknown argument %s\n", argv[i]);
    print_usage();
  }
  verify_inputs();

   // open existing client_q_sem, cashier_array_sem and server_q_sem
  client_q_sem = sem_open(CLIENT_QUEUE_SEM_NAME, 0);
  if (client_q_sem == SEM_FAILED) {
    error("sem_open");
  }
  cashier_array_sem = sem_open(CASHIER_QUEUE_SEM_NAME, 0);
  if (cashier_array_sem == SEM_FAILED) {
    error("sem_open");
  }
  server_q_sem = sem_open(SERVER_QUEUE_SEM_NAME, 0);
  if (server_q_sem == SEM_FAILED) {
    error("sem_open");
  }

  // attach to shm
  mem = (int *) shmat(shmid, (void *)0, 0);
  if (mem == (int *)(-1)) {
    error("shmat");
  }

  char *int_offset = (char *) (mem + SHM_INT_OFFSET + num_cashiers);
  db_offset = (client_db_entry *) (int_offset + num_cashiers * C_INFO_SIZE + S_INFO_SIZE);

  // start of client's time in diner
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double tic = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;

  join_client_queue();
  int cashier_index = take_a_cashier();
  printf("talking to cashier %d...\n", cashier_index);
  // update cashier_client_info struct
  cinfo = (cashier_client_info *) (int_offset + cashier_index * C_INFO_SIZE);
  cinfo->client_pid = getpid();
  cinfo->order_item_id = itemid;
  cinfo->db_index = db_index;
  //printf("%d, %d\n", cinfo->client_pid, cinfo->order_item_id);
  if (sem_init(&(cinfo->client_sem), 1, 0) == -1) { // init value to 0
    error("sem_init");
  }
  // post on cashier_sem to contact
  sem_post(&(cinfo->cashier_sem));
  // wait for the cashier to finish their job and respond
  sem_wait(&(cinfo->client_sem));

  printf("waiting on server...\n");
  // wait on server-queue sem
  sem_wait(server_q_sem);
  printf("talking to server...\n");
  // update server_client_info struct, init server_client_info->client_sem
  sinfo = (server_client_info *) (int_offset + num_cashiers * C_INFO_SIZE);
  sinfo->client_pid = getpid();
  sinfo->order_item_id = itemid;
  sinfo->db_index = db_index;
  if (sem_init(&(sinfo->client_sem), 1, 0) == -1) { // init value to 0
    error("sem_init");
  }
  // post on server_client_info->server_sem
  sem_post(&(sinfo->server_sem));
  // wait on server_client_info->client_sem
  sem_wait(&(sinfo->client_sem));
  // "eat" (sleep) for randomly selected amount of time in [1, max_eat_time]
  printf("eating time...\n");
  sleep(random_in_range(1, max_eat_time));

  // calculate total time spent in shop and enter to database file
  gettimeofday(&tv, NULL);
  double toc = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
  double duration = 1000.0 * (toc - tic) / CLOCKS_PER_SEC;
  write_total_time_to_db(duration);

  // clean up (close sem/detach shm) before exit
  sem_destroy(&(cinfo->client_sem));
  sem_destroy(&(sinfo->client_sem));
  clean_up(client_q_sem, cashier_array_sem, server_q_sem, mem);
}