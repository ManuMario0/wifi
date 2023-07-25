//
//  CSV_common.h
//  wifi
//
//  Created by Emmanuel Mera on 08/06/2023.
//

#ifndef CSV_common_h
#define CSV_common_h

#include <stdio.h>

#include "KER_table.h"

// interpret each column as :
enum {
    CSV_LONG,   // a long integer
    CSV_FLOAT,  // a float
    CSV_HEX,    // a hexadecimal number (no longer than 64 bits long)
    CSV_STR,    // a string (save a copy of the string, can be very memory consuming)
    CSV_AGGLO,  // a hashtable (each entry is saved as a string \
                    and represented by a key, making the key reusable \
                    -> more efficient than CSV_STR when you have low entropy)
    CSV_DATE,   // a date with ISO format (look in the code, not portable)
    CSV_SKIP    // don't store the field
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
    char **reverse_tbl;
} CSV_type;

typedef struct {
    union {
        long l;
        char *str;
        CSV_date *date;
    };
} CSV_cell;

typedef struct {
    char *      file_name;
    
    long        row_count, column_count;
    char **     head;
    CSV_cell *  data;
    CSV_type *  types;
} CSV_file;

/*
 Parse a CSV file (currently not portable)
 */
extern CSV_file *CSV_parse(char file_name[], int *flags, int flags_length);

/*
 Get the row numbre 'row'
 */
extern CSV_cell *CSV_get_row(CSV_file *f, long row);

/*
 Get the original name associated with a key (works only with CSV_AGGLO flag)
 */
extern char *CSV_reverse_id(CSV_file *f, long column, long uid);

#endif /* CSV_common_h */
