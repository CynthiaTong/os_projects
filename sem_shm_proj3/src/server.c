#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "util.h"

/* indices of the two field in menu file */
#define ITEM_PRICE 2
#define ITEM_PREP_TIME_MIN 3
#define ITEM_PREP_TIME_MAX 4

int shmid = -1;
int num_cashiers = -1;
int *mem;
sem_t *server_q_sem;
server_client_info *sinfo;
client_db_entry *db_offset;

/* On user keyboard interrupt, clean up and end this current process */
void sigint_handler() {
  signal(SIGINT, sigint_handler);
  sem_destroy(&(sinfo->server_sem));
  sem_close(server_q_sem);
  detach_shm(mem);
  printf("Bye\n");
  exit(0);
}

void write_order_to_db(int client_pid, double item_price, int food_prep_time, int db_index) {
  client_db_entry *db_entry = db_offset + db_index;
  db_entry->order_item_price = item_price;
  db_entry->order_item_preptime = food_prep_time;
}

void get_prep_time(int item_id, int *prep_time, double *item_price) {
  FILE *fp = fopen(MENU_FILENAME, "r");
  if (fp == NULL) {
    error("failed to open menu file");
  }
  // go to the item_id line in menu file and get a random prep time
  char line[256]; // 256 is large enough for each line
  int line_num = 1;
  while (fgets(line, sizeof(line), fp) != NULL) {
    if (line_num == item_id) {
      char *delim = " ";
      char *token = strtok(line, delim); // parse the line by spaces
      int word_index = 0;
      int min_prep, max_prep;
      while (token) {
        if (word_index == ITEM_PRICE) {
          *item_price = atof(token);
        } else if (word_index == ITEM_PREP_TIME_MIN) {
          min_prep = atoi(token);
        } else if (word_index == ITEM_PREP_TIME_MAX) {
          max_prep = atoi(token);
        }
        token = strtok(NULL, delim);
        ++word_index;
      }
      *prep_time = random_in_range(min_prep, max_prep);
      break;
    } else {
      ++line_num;
    }
  }
}

int print_usage() {
  printf("Usage: ./server -m shmid -n num_cashiers\n");
  exit(1);
}

int main(int argc, char **argv) {
  int opt;
  while ((opt = getopt(argc, argv, ":n:m:")) != -1) {
    switch(opt) {
      case 'n':
        num_cashiers = atoi(optarg);
        break;
      case 'm':
        shmid = atoi(optarg);
        //printf("shmid is %d\n", shmid);
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
  if (shmid == -1 || num_cashiers == -1) {
    print_usage();
  }

  signal(SIGINT, sigint_handler); // set up sig handler for user keyboard interrupt

  // open existing server_q_sem
  server_q_sem = sem_open(SERVER_QUEUE_SEM_NAME, 0);
  if (server_q_sem == SEM_FAILED) {
    error("sem_open");
  }
  // attach to shm
  mem = (int *) shmat(shmid, (void *)0, 0);
  if (mem == (int *)(-1)) {
    error("shmat");
  }
  // init server_client_info->server_sem
  char *int_offset = (char *) (mem + SHM_INT_OFFSET + num_cashiers);
  sinfo = (server_client_info *) (int_offset + num_cashiers * C_INFO_SIZE);
  if (sem_init(&(sinfo->server_sem), 1, 0) == -1) { // init value to 0
    error("sem_init");
  }

  int item_ordered;
  int client_pid;
  int db_index;
  int food_prep_time;
  double item_price;

  db_offset = (client_db_entry *) (int_offset + num_cashiers * C_INFO_SIZE + S_INFO_SIZE);

  while (1) {
    // wait on sinfo->server_sem
    sem_wait(&(sinfo->server_sem));
    client_pid = sinfo->client_pid;
    item_ordered = sinfo->order_item_id;
    db_index = sinfo->db_index;
    get_prep_time(item_ordered, &food_prep_time, &item_price);
    // look up the menu file, then write food price data and food preparation time to db
    //printf("%d %f\n", food_prep_time, item_price);
    write_order_to_db(client_pid, item_price, food_prep_time, db_index);
    // sleep for food preparation time
    printf("food prep will take %d secs...\n", food_prep_time);
    sleep(food_prep_time);
    // post on sinfo->client_sem
    sem_post(&(sinfo->client_sem));
    // post on server-queue sem
    sem_post(server_q_sem);
    printf("served client %d\n", client_pid);
  }

}
