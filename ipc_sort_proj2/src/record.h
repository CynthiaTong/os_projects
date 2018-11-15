#ifndef RECORD_H
#define RECORD_H

typedef struct data_record {
    int taxation_number;
    char first_name[25];
    char last_name[35];
    float income;
} record;

#endif