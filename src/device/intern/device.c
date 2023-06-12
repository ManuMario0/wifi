//
//  device.c
//  empty
//
//  Created by Emmanuel Mera on 08/06/2023.
//

#include <string.h>
#include <limits.h>
#include <time.h>
#include <math.h>

#include "device.h"

#include "DEVICE_common.h"
#include "MEM_alloc.h"
#include "KER_stack.h"

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

long find_floor_index(Device *d, time_t ts);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

DEVICE_device_list *DEVICE_create_device_list(CSV_file *f) {
    DEVICE_device_list *dl = MEM_malloc(sizeof(DEVICE_device_list), __func__);
    
    dl->AP_graph = DEVICE_compute_AP_graph(f);
    dl->AP_count = f->types[APMAC].item_count;
    dl->device_count = f->types[MAC].item_count;
    dl->devices = MEM_malloc_array(sizeof(Device), f->types[MAC].item_count, __func__);
    
    for (int i=0; i<dl->device_count; i++) {
        Device *d = &dl->devices[i];
        
        d->logs_count = 0;
        d->local_csv = MEM_malloc(sizeof(CSV_file), __func__);
        d->local_csv->types = f->types;
        d->local_csv->head = f->head;
        d->local_csv->column_count = f->column_count;
    }
    
    CSV_date *current_date = MEM_calloc_array(dl->device_count, sizeof(CSV_date), __func__);
    
    long *total_days = MEM_calloc_array(dl->device_count, sizeof(long), __func__);
    long *encoutered_AP = MEM_calloc_array(dl->device_count, sizeof(long), __func__);
    long *AP_swap = MEM_calloc_array(dl->device_count, sizeof(long), __func__);
    long *last_AP = MEM_calloc_array(dl->device_count, sizeof(long), __func__);
    
    char **AP_list = MEM_calloc_array(dl->device_count, sizeof(char*), __func__);
    
    for (int i=0; i<dl->device_count; i++) {
        AP_list[i] = MEM_calloc(f->types[APMAC].item_count+1, __func__);
    }
    
    for (int i=0; i<f->row_count; i++) {
        CSV_cell *row = &f->data[i*f->column_count];
        
        if (current_date[row[MAC].l-1].year != row[DATE].date->year
            || current_date[row[MAC].l-1].month != row[DATE].date->month
            || current_date[row[MAC].l-1].day != row[DATE].date->day) {
            
            memset(AP_list[row[MAC].l-1], 0, f->types[APMAC].item_count+1);
            last_AP[row[MAC].l-1] = 0;
            current_date[row[MAC].l-1].year = row[DATE].date->year;
            current_date[row[MAC].l-1].month = row[DATE].date->month;
            current_date[row[MAC].l-1].day = row[DATE].date->day;
            total_days[row[MAC].l-1]++;
        }
        
        if (row[APMAC].l != last_AP[row[MAC].l-1])
            AP_swap[row[MAC].l-1]++;
        
        last_AP[row[MAC].l-1] = row[APMAC].l;
        if (!AP_list[row[MAC].l-1][last_AP[row[MAC].l-1]]) {
            AP_list[row[MAC].l-1][last_AP[row[MAC].l-1]] = 1;
            encoutered_AP[row[MAC].l-1]++;
        }
        dl->devices[row[MAC].l-1].logs_count++;
        dl->devices[row[MAC].l-1].uid = row[UID].l;
    }
    
    for (int i=0; i<dl->device_count; i++) {
        Device *d = &dl->devices[i];
        
        d->average_AP_per_day = (float)encoutered_AP[i]/(float)total_days[i];
        d->average_changes_per_day = (float)AP_swap[i]/(float)total_days[i];
        d->mac = i;
        
        if (d->average_AP_per_day > 5 && d->average_changes_per_day > 8)
            d->type = MOBILE;
        else if (d->average_AP_per_day < 5 && d->average_changes_per_day < 8)
            d->type = FIXE;
        else
            d->type = UNKNOWN;
        
        d->local_csv->row_count = d->logs_count;
        d->local_csv->data = MEM_malloc_array(sizeof(CSV_cell), d->local_csv->row_count*d->local_csv->column_count, __func__);
    }
    
    MEM_free(current_date);
    MEM_free(total_days);
    MEM_free(encoutered_AP);
    MEM_free(AP_swap);
    MEM_free(last_AP);
    
    for (int i=0; i<dl->device_count; i++) {
        MEM_free(AP_list[i]);
    }
    MEM_free(AP_list);
    
    long *index = MEM_calloc_array(dl->device_count, sizeof(long), __func__);
    for (long i=f->row_count-1; i>=0; i--) {
        CSV_cell *row = &f->data[i*f->column_count];
        
        if (index[row[MAC].l-1] == 0) {
            dl->devices[row[MAC].l-1].start_time = row[TIMESTAMP].l;
        }
        
        memcpy(&dl->devices[row[MAC].l-1].local_csv->data[index[row[MAC].l-1]*f->column_count], row, f->column_count*sizeof(CSV_cell));
        index[row[MAC].l-1]++;
        
        if (index[row[MAC].l-1] == dl->devices[row[MAC].l-1].logs_count) {
            dl->devices[row[MAC].l-1].end_time = row[TIMESTAMP].l;
        }
    }
    MEM_free(index);
    
    return dl;
}

float *DEVICE_compute_AP_graph(CSV_file *f) {
    long AP_count = f->types[APMAC].item_count+1;
    long *distances = MEM_calloc_array(AP_count*AP_count, sizeof(long), __func__);
    long *counts = MEM_calloc_array(AP_count*AP_count, sizeof(long), __func__);
    
    for (int mac=1; mac<=f->types[MAC].item_count; mac++) {
        long current_date = LONG_MAX;
        long current_AP = 0;
        for (int i=0; i<f->row_count; i++) {
            CSV_cell *row = &f->data[i*f->column_count];
            
            if (row[MAC].l == mac) {
                if (current_date - row[TIMESTAMP].l < 10 && row[APMAC].l != current_AP && current_date - row[TIMESTAMP].l>=0) {
                    distances[row[APMAC].l * AP_count + current_AP] += current_date - row[TIMESTAMP].l;
                    counts[row[APMAC].l * AP_count + current_AP] += 1;
                    
                    distances[row[APMAC].l + AP_count * current_AP]
                                    = distances[row[APMAC].l * AP_count + current_AP];
                    counts[row[APMAC].l + AP_count * current_AP]
                                    = counts[row[APMAC].l * AP_count + current_AP];
                }
                current_date = row[TIMESTAMP].l;
                current_AP = row[APMAC].l;
            }
        }
    }
    
    float *dist = MEM_calloc_array(AP_count*AP_count, sizeof(float), __func__);
    for (int i=1; i<=f->types[APMAC].item_count; i++) {
        for (int j=1; j<=f->types[APMAC].item_count; j++) {
            if (counts[i * AP_count + j] != 0) {
                dist[i * AP_count + j]
                        = (float)distances[i * AP_count + j] / (float)counts[i * AP_count + j];
            } else {
                dist[i * AP_count + j] = 1000.;
            }
        }
    }
    
    MEM_free(distances);
    MEM_free(counts);
    
    // Roy-Warshall-Floyd
    for (int i=1; i<=f->types[APMAC].item_count; i++) {
        dist[i * AP_count + i] = 0.;
    }
    
    for (int k=1; k<=f->types[APMAC].item_count; k++) {
        for (int i=1; i<=f->types[APMAC].item_count; i++) {
            for (int j=1; j<=f->types[APMAC].item_count; j++) {
                dist[i * AP_count + j] = MIN(dist[i * AP_count + j], dist[i * AP_count + k] + dist[k * AP_count + j]);
            }
        }
    }
    
    return dist;
}

float DEVICE_get_AP_distance(DEVICE_device_list *dl, long AP1, long AP2) {
    return dl->AP_graph[AP1 * (dl->AP_count+1) + AP2];
}

// later, we need the ap proximity graph
float DEVICE_proximity(Device *d1, Device *d2, CSV_date start, CSV_date end, DEVICE_device_list *dl) {
    long stop_signal = (long)KER_hash_find(d1->local_csv->types[STATUS_TYPE].tbl, "Stop", 4);
    
    struct tm start_tm;
    struct tm end_tm;
    
    start_tm.tm_year = start.year - 1900;
    start_tm.tm_mon = start.month - 1;
    start_tm.tm_mday = start.day;
    start_tm.tm_hour = start.hour;
    start_tm.tm_min = start.min;
    start_tm.tm_sec = start.sec;
    start_tm.tm_isdst = 1;
    
    end_tm.tm_year = end.year - 1900;
    end_tm.tm_mon = end.month - 1;
    end_tm.tm_mday = end.day;
    end_tm.tm_hour = end.hour;
    end_tm.tm_min = end.min;
    end_tm.tm_sec = end.sec;
    end_tm.tm_isdst = 1;
    
    time_t start_ts = mktime(&start_tm);
    time_t end_ts = mktime(&end_tm);
    
    
    long index_d1 = find_floor_index(d1, start_ts);
    long index_d2 = find_floor_index(d2, start_ts);
    
    long AP_d1 = -1;
    long AP_d2 = -1;
    
    CSV_cell *row_d1;
    CSV_cell *row_d2;
    
    if (index_d1 >= 0) {
        row_d1 = &d1->local_csv->data[index_d1*d1->local_csv->column_count];
        if (row_d1[STATUS_TYPE].l != stop_signal)
            AP_d1 = row_d1[APMAC].l;
    }
    
    if (index_d2 >= 0) {
        row_d2 = &d2->local_csv->data[index_d2*d2->local_csv->column_count];
        if (row_d2[STATUS_TYPE].l != stop_signal)
            AP_d2 = row_d2[APMAC].l;
    }
    
    long current_time = start_ts;
    
    long total_time = 0;
    float distance = 0.;
    
    index_d1++; index_d2++;
    
    if (index_d1 < d1->local_csv->row_count)
        row_d1 = &d1->local_csv->data[index_d1*d1->local_csv->column_count];
    else
        row_d1 = NULL;
    
    if (index_d2 < d2->local_csv->row_count)
        row_d2 = &d2->local_csv->data[index_d2*d2->local_csv->column_count];
    else
        row_d2 = NULL;
    
    while (current_time < end_ts) {
        if (index_d1 < d1->local_csv->row_count && index_d2 < d2->local_csv->row_count) {
            float dist;
            if (AP_d1 != -1 && AP_d2 != -1)
                dist = DEVICE_get_AP_distance(dl, AP_d1, AP_d2);
            else
                dist = 1000.;
            
            if (row_d1[TIMESTAMP].l < row_d2[TIMESTAMP].l) {
                if (AP_d1 != -1 && AP_d2 != -1) {
                    distance += dist * (MIN(row_d1[TIMESTAMP].l, end_ts) - current_time);
                    total_time += MIN(row_d1[TIMESTAMP].l, end_ts) - current_time;
                }
                
                current_time = MIN(row_d1[TIMESTAMP].l, end_ts);
                AP_d1 = row_d1[APMAC].l;
                if (row_d1[STATUS_TYPE].l == stop_signal || AP_d1 == 2)
                    AP_d1 = -1;
                index_d1 ++;
                if (index_d1 < d1->local_csv->row_count) {
                    row_d1 = &d1->local_csv->data[index_d1*d1->local_csv->column_count];
                }
            } else {
                if (AP_d1 != -1 && AP_d2 != -1) {
                    distance += dist * (MIN(row_d2[TIMESTAMP].l, end_ts) - current_time);
                    total_time += MIN(row_d2[TIMESTAMP].l, end_ts) - current_time;
                }
                
                current_time = MIN(row_d2[TIMESTAMP].l, end_ts);
                AP_d2 = row_d2[APMAC].l;
                if (row_d2[STATUS_TYPE].l == stop_signal || AP_d2 == 2)
                    AP_d2 = -1;
                index_d2 ++;
                if (index_d2 < d2->local_csv->row_count) {
                    row_d2 = &d2->local_csv->data[index_d2*d2->local_csv->column_count];
                }
            }
        } else if (index_d1 == d1->local_csv->row_count || index_d2 == d2->local_csv->row_count) {
            current_time = end_ts;
        } else if (index_d1 == d1->local_csv->row_count) {
            if (AP_d2 != -1) {
                distance += 1000. * (MIN(row_d2[TIMESTAMP].l, end_ts) - current_time);
                total_time += MIN(row_d2[TIMESTAMP].l, end_ts) - current_time;
            }
            
            current_time = MIN(row_d2[TIMESTAMP].l, end_ts);
            AP_d2 = row_d2[APMAC].l;
            if (row_d2[STATUS_TYPE].l == stop_signal)
                AP_d2 = -1;
            index_d2 ++;
            if (index_d2 < d2->local_csv->row_count) {
                row_d2 = &d2->local_csv->data[index_d2*d2->local_csv->column_count];
            }
        } else {
            if (AP_d1 != -1) {
                distance += 1000. * (MIN(row_d1[TIMESTAMP].l, end_ts) - current_time);
                total_time += MIN(row_d1[TIMESTAMP].l, end_ts) - current_time;
            }
            
            current_time = MIN(row_d1[TIMESTAMP].l, end_ts);
            AP_d1 = row_d1[APMAC].l;
            if (row_d1[STATUS_TYPE].l == stop_signal)
                AP_d1 = -1;
            index_d1 ++;
            if (index_d1 < d1->local_csv->row_count) {
                row_d1 = &d1->local_csv->data[index_d1*d1->local_csv->column_count];
            }
        }
    }
    
    if (total_time != 0/* && total_time > 3*(float)(end_ts-start_ts)/10.*/)
        return distance/(float)total_time;
    else
        return INFINITY;
}

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

long find_floor_index(Device *d, time_t ts) {
    long left = 0;
    long right = d->local_csv->row_count-1;
    CSV_cell *row;
    
    while (left < right) {
        long m = (right+left+1)/2;
        row = &d->local_csv->data[m*d->local_csv->column_count];
        if (row[TIMESTAMP].l < ts)
            left = m;
        else
            right = m-1;
    }
    
    row = &d->local_csv->data[left*d->local_csv->column_count];
    if (row[TIMESTAMP].l > ts)
        return --left;
    return left;
}
