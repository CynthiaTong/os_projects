#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h> // for sleep syscall
#include <signal.h>

#include "common.h"
#include "util.h"

int *mem;
int max_service_time = -1;
int max_break_time = -1;
int shmid = -1;
int cashier_index = -1;
int num_cashiers = -1;

sem_t *client_q_sem;
sem_t *cashier_array_sem;
cashier_client_info *cinfo; // cashier_client_info struct for this particular cashier
client_db_entry *db_offset;

int check_num_clients_inline() {
  return mem[0];
}

void update_num_clients() {
  sem_wait(client_q_sem);
  --mem[0]; // decrement num clients in line
  sem_post(client_q_sem);
}

int check_available_flag() {
  int available_flag = mem[SHM_INT_OFFSET + cashier_index];
  return available_flag;
}

void set_available_flag() {
  sem_wait(cashier_array_sem);
  mem[SHM_INT_OFFSET + cashier_index] = 1;
  sem_post(cashier_array_sem);
}

void write_new_record(int client_id, int item_id, int db_index) {
  client_db_entry *db_entry = db_offset + db_index;
  db_entry->client_pid = client_id;
  db_entry->order_item_id = item_id;
}

/* On user keyboard interrupt, clean up and end this current process */
void sigint_handler() {
  signal(SIGINT, sigint_handler);
  sem_destroy(&(cinfo->cashier_sem));
  sem_close(client_q_sem);
  sem_close(cashier_array_sem);
  detach_shm(mem);
  printf("Bye\n");
  exit(0);
}

int print_usage() {
  printf("Usage: ./cashier -s serviceTime -b breakTime -m shmid -i shmIndex -n numCashiers\n");
  exit(1);
}

void verify_inputs() {
  if (max_service_time == -1 || max_break_time == -1 ||
    shmid == -1 || cashier_index == -1 || num_cashiers == -1) {
    print_usage();
  }
}

int main(int argc, char **argv) {
  srand(time(NULL));    // random seed

  // take and verify user input
  int opt;
  while ((opt = getopt(argc, argv, ":s:b:m:i:n:")) != -1) {
    switch(opt) {
      case 's':
        max_service_time = atoi(optarg);
        //printf("max_service_time: %d\n", max_service_time);
        break;
      case 'b':
        max_break_time = atoi(optarg);
        //printf("max_break_time %d\n", max_break_time);
        break;
      case 'm':
        shmid = atoi(optarg);
        //printf("shmid is %d\n", shmid);
        break;
      case 'i':
        cashier_index = atoi(optarg);
        //printf("cashier_index is %d\n", cashier_index);
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

  signal(SIGINT, sigint_handler); // set up sig handler for user keyboard interrupt

  // open existing client_q_sem and cashier_array_sem
  client_q_sem = sem_open(CLIENT_QUEUE_SEM_NAME, 0);
  if (client_q_sem == SEM_FAILED) {
    error("sem_open");
  }
  cashier_array_sem = sem_open(CASHIER_QUEUE_SEM_NAME, 0);
  if (cashier_array_sem == SEM_FAILED) {
    error("sem_open");
  }

  // attach to shm
  mem = (int *) shmat(shmid, (void *)0, 0);
  if (mem == (int *)(-1)) {
    error("shmat");
  }

  // init corresponding cashier_client_info
  char *int_offset = (char *) (mem + SHM_INT_OFFSET + num_cashiers);
  cinfo =(cashier_client_info *) (int_offset + cashier_index * C_INFO_SIZE);
  if (sem_init(&(cinfo->cashier_sem), 1, 0) == -1) { // init value to 0
    error("sem_init");
  }

  db_offset = (client_db_entry *) (int_offset + num_cashiers * C_INFO_SIZE + S_INFO_SIZE);
  /* start to communicate with incoming client processes */
  while (1) {
    while (check_available_flag()) {
      if (check_num_clients_inline() == 0) {
        printf("cashier %d taking a break...\n", cashier_index);
        sleep(random_in_range(1, max_break_time));
      }
    }
    // if a client changed the availability flag to 0, wait on cashier_sem
    sem_wait(&(cinfo->cashier_sem));
    printf("cashier %d serving client %d\n", cashier_index, cinfo->client_pid);
    // read client info from shm and append it to end of record txt file
    write_new_record(cinfo->client_pid, cinfo->order_item_id, cinfo->db_index);
    // sleep for random[1, service_time]
    sleep(random_in_range(1, max_service_time));
    // post to client_sem to signal the cashier's job is done
    sem_post(&(cinfo->client_sem));
    // flip the availability bit in shm and decrement number of clients in queue
    set_available_flag();
    update_num_clients();
  }
}