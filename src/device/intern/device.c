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
#include <ctype.h>

#include "device.h"

#include "DEVICE_common.h"
#include "MEM_alloc.h"
#include "KER_stack.h"

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static long find_floor_index(Device *d, time_t ts);
static float *load_graph(FILE *fp, long size);
static void write_graph(FILE *fp, float *graph, long size);
static char *build_graph_name(CSV_file *f);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

DEVICE_device_list *DEVICE_create_device_list(char filename[]) {
    int flags[] = {CSV_DATE, CSV_LONG, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_AGGLO, CSV_SKIP, CSV_SKIP, CSV_AGGLO};
    CSV_file *f = CSV_parse(filename, flags, sizeof(flags));
    
    DEVICE_device_list *dl = MEM_calloc(sizeof(DEVICE_device_list), __func__);
    
    dl->AP_graph = DEVICE_compute_AP_graph(f);
    dl->AP_count = f->types[APMAC].item_count;
    dl->device_count = f->types[MAC].item_count;
    dl->devices = MEM_calloc_array(sizeof(Device), f->types[MAC].item_count, __func__);
    dl->csv = f;
    
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
    
    long evil_AP = (long)KER_hash_find(dl->devices->local_csv->types[APMAC].tbl, "\0", 0);
    
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
        if (row[APMAC].l != evil_AP)
            dl->devices[row[MAC].l-1].logs_count++;
        else
            dl->devices[row[MAC].l-1].skiped_logs++;
        dl->devices[row[MAC].l-1].uid = row[UID].l-1;
    }
    
    for (int i=0; i<dl->device_count; i++) {
        Device *d = &dl->devices[i];
        
        d->average_AP_per_day = (float)encoutered_AP[i]/(float)total_days[i];
        d->average_changes_per_day = (float)AP_swap[i]/(float)total_days[i];
        d->mac = i;
        
        if (d->average_AP_per_day > 5 && d->average_changes_per_day > 8) {
            d->type = MOBILE;
            dl->total_mobile++;
        }
        else if (d->average_AP_per_day < 5 && d->average_changes_per_day < 8) {
            d->type = STATIC;
            dl->total_fixe++;
        }
        else {
            d->type = UNKNOWN;
            dl->total_unknown++;
        }
        
        d->local_csv->row_count = d->logs_count;
        d->local_csv->data = MEM_malloc_array(sizeof(CSV_cell), d->local_csv->row_count*d->local_csv->column_count, __func__);
        
        if (d->logs_count != 0)
            dl->effective_device_count++;
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
        
        if (row[APMAC].l != evil_AP) {
            memcpy(&dl->devices[row[MAC].l-1].local_csv->data[index[row[MAC].l-1]*f->column_count], row, f->column_count*sizeof(CSV_cell));
            index[row[MAC].l-1]++;
        }
        
        if (index[row[MAC].l-1] == dl->devices[row[MAC].l-1].logs_count) {
            dl->devices[row[MAC].l-1].end_time = row[TIMESTAMP].l;
        }
    }
    MEM_free(index);
    
    return dl;
}

float *DEVICE_compute_AP_graph(CSV_file *f) {
    char *saved_file_name = build_graph_name(f);
    
    FILE *fp = fopen(saved_file_name, "r+");
    MEM_free(saved_file_name);
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (size > 0) {
        float *res = load_graph(fp, (f->types[APMAC].item_count+1)*(f->types[APMAC].item_count+1));
        fclose(fp);
        return res;
    }
    
    long AP_count = f->types[APMAC].item_count+1;
    long *distances = MEM_calloc_array(AP_count*AP_count, sizeof(long), __func__);
    long *counts = MEM_calloc_array(AP_count*AP_count, sizeof(long), __func__);
    
    for (long mac=1; mac<=f->types[MAC].item_count; mac++) {
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
            if (counts[i * AP_count + j] > 30) {
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
    
    write_graph(fp, dist, AP_count*AP_count);
    
    fclose(fp);
    
    return dist;
}

float DEVICE_get_AP_distance(DEVICE_device_list *dl, long AP1, long AP2) {
    return dl->AP_graph[AP1 * (dl->AP_count+1) + AP2];
}

float DEVICE_proximity(Device *d1, Device *d2, time_t start_ts, time_t end_ts, DEVICE_device_list *dl) {
    long stop_signal = (long)KER_hash_find(d1->local_csv->types[STATUS_TYPE].tbl, "Stop", 4);
    
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
                if (row_d1[STATUS_TYPE].l == stop_signal)
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
                if (row_d2[STATUS_TYPE].l == stop_signal)
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
    
    if (total_time != 0)
        return distance/(float)total_time;
    else
        return INFINITY;
}

void DEVICE_print_devices_stats(DEVICE_device_list *dl) {
    printf("Total devices detected : %ld\n", dl->device_count);
    printf("Total mobiles : %ld\nTotal fixe : %ld\nTotal unknown : %ld\n", dl->total_mobile, dl->total_fixe, dl->total_unknown);
    printf("Total AP detected : %ld\n", dl->AP_count);
    printf("   TYPE      LOGS   SKIPED        START          END     UID     MAC                                                        REAL-MAC\n");
    
    for (int i=0; i<dl->device_count; i++) {
        switch (dl->devices[i].type) {
            case MOBILE:
                printf(" MOBILE   ");
                break;
                
            case STATIC:
                printf(" STATIC   ");
                break;
                
            case UNKNOWN:
                printf("UNKNOWN   ");
                break;
        }
        
        printf("%7ld   ", dl->devices[i].logs_count);
        
        printf("%6ld   ", dl->devices[i].skiped_logs);
        
        char buffer[2048];
        strftime(buffer, 2048, "%F", localtime(&dl->devices[i].start_time));
        
        printf("%s   ", buffer);
        
        strftime(buffer, 2048, "%F", localtime(&dl->devices[i].end_time));
        printf("%s   ", buffer);
        
        printf("%5ld   ", dl->devices[i].uid);
        printf("%5d   ", dl->devices[i].mac);
        printf("%s", CSV_reverse_id(dl->csv, MAC, dl->devices[i].mac+1));
        
        printf("\n");
    }
}

void DEVICE_store_graph(DEVICE_device_list *dl, char *filename) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "graph {\n");
        
        for (int i=1; i<=dl->AP_count; i++) {
            for (int j=i+1; j<=dl->AP_count; j++) {
                if (dl->AP_graph[i*dl->AP_count+j] != 1000.)
                    fprintf(f, "%d -- %d[len=%f];\n", i, j, dl->AP_graph[i*dl->AP_count+j]);
            }
        }
        
        fprintf(f, "}");
        
        fclose(f);
    }
}

// TODO: add bound checking when in debug mode
inline CSV_cell *DEVICE_get_row(Device *d, long index) {
    return CSV_get_row(d->local_csv, index);
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
        row =  DEVICE_get_row(d, m);
        if (row[TIMESTAMP].l < ts)
            left = m;
        else
            right = m-1;
    }
    
    row = DEVICE_get_row(d, left);
    if (row[TIMESTAMP].l > ts)
        return --left;
    return left;
}

float *load_graph(FILE *fp, long size) {
    char *buffer = MEM_malloc_array(size, sizeof(float), __func__);
    
    fread(buffer, sizeof(float), size, fp);
    
    return (float*)buffer;
}

void write_graph(FILE *fp, float *graph, long size) {
    fwrite(graph, sizeof(float), size, fp);
}

char *build_graph_name(CSV_file *f) {
    char *saved_file_name = MEM_malloc(strlen(f->file_name)+10, __func__);
    long index;
    for (index=strlen(f->file_name)-1; index>=0; index--) {
        if (f->file_name[index] == '/')
            break;
    }
    
    memcpy(saved_file_name, f->file_name, index+1);
    saved_file_name[index+1] = '.';
    memcpy(&saved_file_name[index+2], &f->file_name[index+1], strlen(f->file_name)-1-index);
    memcpy(&saved_file_name[strlen(f->file_name)+1], ".graph", strlen(".graph")+1);
    
    return saved_file_name;
}
