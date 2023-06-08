//
//  device.c
//  empty
//
//  Created by Emmanuel Mera on 08/06/2023.
//

#include <string.h>

#include "device.h"
#include "MEM_alloc.h"
#include "CSV_common.h"

Device *create_device(int mac, CSV_file *f) {
    Device *d = MEM_calloc(sizeof(Device), __func__);
    
    CSV_date current_date;
    current_date.year = current_date.month = current_date.day = 0;
    
    long total_days = 0;
    long encoutered_AP = 0;
    long AP_swap = 0;
    long last_AP = 0;
    
    char *AP_list = MEM_calloc(f->types[APMAC].item_count+1, __func__);
    
    for (int i=0; i<f->row_count; i++) {
        CSV_cell *row = &f->data[i*f->column_count];
        
        if (row[MAC].l == mac) {
            if (current_date.year != row[DATE].date->year
                || current_date.month != row[DATE].date->month
                || current_date.day != row[DATE].date->day) {
                
                memset(AP_list, 0, f->types[APMAC].item_count+1);
                last_AP = 0;
                current_date.year = row[DATE].date->year;
                current_date.month = row[DATE].date->month;
                current_date.day = row[DATE].date->day;
                total_days++;
            }
            
            if (row[APMAC].l != last_AP)
                AP_swap++;
            
            last_AP = row[APMAC].l;
            if (!AP_list[last_AP]) {
                AP_list[last_AP] = 1;
                encoutered_AP++;
            }
            d->logs_count++;
        }
    }
    
    d->average_AP_per_day = (float)encoutered_AP/(float)total_days;
    d->average_changes_per_day = (float)AP_swap/(float)total_days;
    
    return d;
}
