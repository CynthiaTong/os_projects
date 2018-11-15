#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <sys/types.h>  // for pid_t type
#include <semaphore.h>

typedef struct cashier_info {
  pid_t client_pid;
  int order_item_id;
  int db_index;
  sem_t cashier_sem;
  sem_t client_sem;
} cashier_client_info;

typedef struct server_info {
  pid_t client_pid;
  int order_item_id;
  int db_index;
  sem_t server_sem;
  sem_t client_sem;
} server_client_info;

typedef struct client_entry {
  pid_t client_pid;
  int order_item_id;
  double order_item_price;
  int order_item_preptime;
  double client_total_time;
} client_db_entry;

typedef struct item {
  int item_id;
  int times_ordered;
} order_item;

#define SHM_PERMISSION 0666
#define SEM_PERMISSION 0644

#define DATABASE_FILENAME "database.txt"
#define MENU_FILENAME "menu.txt"

#define CLIENT_QUEUE_SEM_NAME "client-queue"
#define CASHIER_QUEUE_SEM_NAME "cashier-queue"
#define SERVER_QUEUE_SEM_NAME "server-queue"

#define DEFAULT_NUM_CASHIERS 3
#define MAX_NUM_CLIENTS_INLINE 7
#define MAX_NUM_CLIENTS_TOTAL 50
#define SHM_INT_OFFSET 2 // first 2 ints in shm are counters for num of clients
#define C_INFO_SIZE sizeof(cashier_client_info)
#define S_INFO_SIZE sizeof(server_client_info)
#define C_ENTRY_SIZE sizeof(client_db_entry)
#define TOTAL_NUM_MENU_ITEMS 20

#endif