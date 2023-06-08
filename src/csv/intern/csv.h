//
//  csv.h
//  empty
//
//  Created by Emmanuel Mera on 05/06/2023.
//

#ifndef csv_h
#define csv_h

#include <stdio.h>

#include "KER_table.h"

enum {
    CSV_LONG,
    CSV_FLOAT,
    CSV_HEX,
    CSV_STR,
    CSV_AGGLO,
    CSV_DATE,
    CSV_SKIP
};

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
} CSV_date;

typedef struct {
    int type;
    long item_count;
    KER_hashTable *tbl;
} CSV_type;

typedef struct {
    union {
        long l;
        char *str;
        CSV_date *date;
    };
} CSV_cell;

typedef struct {
    long        row_count, column_count;
    char **     head;
    CSV_cell *  data;
    CSV_type *  types;
} CSV_file;

#endif /* csv_h */
