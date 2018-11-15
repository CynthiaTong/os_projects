#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "record.h"

#define MERGE_SPLIT "./merge_split"
#define SH "./SH"
#define QS "./QS"
#define BS "./BS"

#define TIMING_DIR "./timing/"

#define MIN_DEPTH 1
#define MAX_DEPTH 6

#define FIRST_NAME_LENGTH 25
#define LAST_NAME_LENGTH 35
#define DATA_RECORD_LENGTH sizeof(struct data_record)
#define FIRST_NAME_OFFSET sizeof(int)
#define LAST_NAME_OFFSET FIRST_NAME_OFFSET + FIRST_NAME_LENGTH
#define INCOME_OFFSET sizeof(struct data_record) - sizeof(float)

#define SORT_ON_TAX 0
#define SORT_ON_FIRST 1
#define SORT_ON_LAST 2
#define SORT_ON_INCOME 3

#define NUM_OF_SORTS 3
#define USE_SHELL_SORT 0
#define USE_QUICK_SORT 1
#define USE_BUBBLE_SORT 2

#define READ 0
#define WRITE 1

#endif