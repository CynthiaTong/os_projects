#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <string.h>
#include <getopt.h>

#include "common.h"
#include "util.h"

int num_cashiers = DEFAULT_NUM_CASHIERS;

int print_usage() {
  printf("Usage: ./coordinator (-m MaxNumberOfCashiers)\n");
  exit(1);
}

int main(int argc, char **argv) {
  // take user argument and change num_cashiers if MaxNumberOfCashiers is present
  int opt;
  while ((opt = getopt(argc, argv, ":m:")) != -1) {
    switch(opt) {
      case 'm':
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
  if (num_cashiers < 0) {
    printf("MaxNumberOfCashiers must be positive.\n");
    print_usage();
  }

  /*
    calculate shared memory size and initialize
    1st int: num_in_client_q
    2nd int: total number of clients so far
    num_cashier number of ints: availability flags
    num_cashier number of client_info
  */
  int shm_size = sizeof(int) * (SHM_INT_OFFSET + num_cashiers) + // int and int arrays
    C_INFO_SIZE * num_cashiers + S_INFO_SIZE +  // cashiers and server data storage
    C_ENTRY_SIZE * MAX_NUM_CLIENTS_TOTAL; // db size

	int shmid = shmget(IPC_PRIVATE, shm_size, SHM_PERMISSION);
  if (shmid == -1) {
    error("shmget");
  }
  printf("shmid: %d\n", shmid);
  //printf("shm size: %d\n", shm_size);
  printf("num cashiers %d\n", num_cashiers);

  // attach to shm
  int *mem = (int *)shmat(shmid, (void *)0, 0);
  if (mem == (int *)(-1)) {
    error("shmat");
  }

  mem[0] = 0; // initially, 0 clients in queue
  mem[1] = 0; // no clients in total
  for (int i = 0; i < num_cashiers; ++i) {
    mem[i + SHM_INT_OFFSET] = 1; // initially, all cashiers are available
  }
  /*
    set up named semaphores for the client queue and cashier queue,
    which are binary sems to protect the update of num_in_client_q
    and the cashier availability array
  */
  sem_t *client_q_sem = initialize_sem(CLIENT_QUEUE_SEM_NAME, 1);
  sem_t *cashier_array_sem = initialize_sem(CASHIER_QUEUE_SEM_NAME, 1);  // incremented by each cashier process
  sem_t *server_q_sem = initialize_sem(SERVER_QUEUE_SEM_NAME, 1);

  // clean up from last run: remove database file if exist
  remove(DATABASE_FILENAME);

  getchar();

  /* output diner operation records to a record file */
  FILE *record = fopen("record.txt", "w+");
  if (record == NULL) {
    error("failed to open record file");
  }

  char *int_offset = (char *) (mem + SHM_INT_OFFSET + num_cashiers);
  client_db_entry *db = (client_db_entry *) (int_offset + num_cashiers * C_INFO_SIZE + S_INFO_SIZE);
  int total_num_clients = mem[1];
  double revenue;
  double avg_waiting_time;

  order_item top_orders[TOTAL_NUM_MENU_ITEMS];
  for (int i = 0; i < TOTAL_NUM_MENU_ITEMS; ++i) {
    top_orders[i].item_id = i + 1;
    top_orders[i].times_ordered = 0;
  }

  for (int i = 0; i < total_num_clients; ++i) {
    fprintf(
      record,
      "pid %d, item %d, price %f, waiting time %d, total time %f\n",
      db->client_pid,
      db->order_item_id,
      db->order_item_price,
      db->order_item_preptime,
      db->client_total_time
    );
    revenue += db->order_item_price;
    avg_waiting_time += db->order_item_preptime;
    ++top_orders[db->order_item_id - 1].times_ordered;  // -1 since 1/0 ordering
    ++db;
  }

  fprintf(record, "***Stats***\n");
  avg_waiting_time /= total_num_clients * 1.0;
  fprintf(record, "Average waiting time: %f\n", avg_waiting_time);
  fprintf(record, "Total number of clients: %d\n", total_num_clients);
  fprintf(record, "Total revenue: %f\n", revenue);
  // qsort to find top 5 most ordered items
  qsort(top_orders, TOTAL_NUM_MENU_ITEMS, sizeof(*top_orders), comp);
  fprintf(record, "Top 5 most popular items (id, times ordered):\n");
  for (int i = TOTAL_NUM_MENU_ITEMS -1; i >= TOTAL_NUM_MENU_ITEMS - 5; --i) {
    fprintf(record, "%d, %d\n", top_orders[i].item_id, top_orders[i].times_ordered);
  }
  fclose(record);

  /* release all resources (shm & sem) */
  // free items in cashier_sem_names
  // free all sems
  detach_shm(mem);
  sem_close(client_q_sem);
  sem_close(cashier_array_sem);
  sem_close(server_q_sem);
  if (shmctl(shmid, IPC_RMID, 0) == -1) {
    error("shmctl remove");
  }
}