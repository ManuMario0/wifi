//
//  user.c
//  empty
//
//  Created by Emmanuel Mera on 09/06/2023.
//

#include <string.h>
#include <time.h>

#include "user.h"

#include "MEM_alloc.h"

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

USR_user_list *USR_create_user_list(DEV_device_list *dl) {
    USR_user_list *ul = MEM_malloc(sizeof(USR_user_list), __func__);
    ul->user_count = dl->devices->local_csv->types[UID].item_count;
    ul->users = MEM_calloc_array(ul->user_count, sizeof(USR_user), __func__);
    
    for (int i=0; i<dl->device_count; i++) {
        DEV_device *d = &dl->devices[i];
        if (d->logs_count > 100 && d->end_time - d->start_time > 604800) {
            ul->users[d->uid].device_count ++;
            ul->users[d->uid].stats[d->type] ++;
        }
    }
    
    for (int i=0; i<ul->user_count; i++) {
        ul->users[i].devices = MEM_malloc_array(ul->users[i].device_count, sizeof(DEV_device*), __func__);
        ul->users[i].device_count = 0;
        ul->users[i].uid = i;
        ul->users[i].real_uid = dl->csv->types[UID].reverse_tbl[i];
    }
    
    for (int i=0; i<dl->device_count; i++) {
        DEV_device *d = &dl->devices[i];
        if (d->logs_count > 100 && d->end_time - d->start_time > 604800) {
            ul->users[d->uid].devices[ul->users[d->uid].device_count] = d;
            ul->users[d->uid].device_count ++;
        }
    }
    
    return ul;
}

USR_relation *USR_create_user_relation_graph(USR_user_list *    ul,
                                             DEV_device_list *dl,
                                             CSV_date           start,
                                             CSV_date           period_length,
                                             int                interval,
                                             CSV_date           record_start,
                                             CSV_date           record_end,
                                             int                filter) {
    USR_relation *rlt = MEM_calloc(sizeof(USR_relation), __func__);
    rlt->start = start;
    rlt->period_length = period_length;
    rlt->interval = interval;
    rlt->record_start = record_start;
    rlt->record_end = record_end;
    
    rlt->relation_graph = MEM_calloc_array(ul->user_count*ul->user_count, sizeof(long), __func__);
    
    switch (interval) {
        case YEAR:
            rlt->start.month = 0;
            
        case MONTH:
            rlt->start.day = 0;
            
        case DAY:
            rlt->start.hour = 0;
            
        case HOUR:
            rlt->start.min = 0;
            rlt->start.sec = 0;
            
        default:
            break;
    }
    
    struct tm end_tm = {0};
    end_tm.tm_mday = end_tm.tm_mon = end_tm.tm_mday = end_tm.tm_hour = 0;
    end_tm.tm_isdst = 1;
    
    switch (interval) {
        case HOUR:
            end_tm.tm_hour = rlt->start.hour + period_length.hour;
            
        case DAY:
            end_tm.tm_mday = rlt->start.day + period_length.day;
            
        case MONTH:
            end_tm.tm_mon = rlt->start.month + period_length.month - 1;
            
        case YEAR:
            end_tm.tm_year = rlt->start.year + period_length.year - 1900;
            
        default:
            break;
    }
    
    time_t end = mktime(&end_tm);
    
    
    struct tm current_date_tm = {0};
    current_date_tm.tm_year = rlt->start.year - 1900;
    current_date_tm.tm_mon = rlt->start.month - 1;
    current_date_tm.tm_mday = rlt->start.day;
    current_date_tm.tm_hour = rlt->start.hour;
    current_date_tm.tm_isdst = 1;
    
    time_t current_date = mktime(&current_date_tm);
    
    time_t record_start_ts = 0;
    time_t record_end_ts = 0;
    
    switch (interval) {
        case YEAR:
            record_start_ts += record_start.month*2629743;
            record_end_ts += record_end.month*2629743;
            
        case MONTH:
            record_start_ts += record_start.day*86400;
            record_end_ts += record_end.day*86400;
            
        case DAY:
            record_start_ts += record_start.hour*3600;
            record_end_ts += record_end.hour*3600;
            
        case HOUR:
            record_start_ts += record_start.min*60;
            record_end_ts += record_end.min*60;
            record_start_ts += record_start.sec;
            record_end_ts += record_end.sec;
            
        default:
            break;
    }
    
    while (current_date < end) {
        for (long k=0; k<ul->user_count; k++) {
            for (long l=k+1; l<ul->user_count; l++) {
                for (long i=0; i<ul->users[k].device_count; i++) {
                    if (ul->users[k].devices[i]->type & filter) {
                        for (long j=0; j<ul->users[l].device_count; j++) {
                            if (ul->users[l].devices[j]->type & filter) {
                                float dist = DEV_proximity(ul->users[k].devices[i], ul->users[l].devices[j], current_date+record_start_ts, current_date+record_end_ts, dl);
                                
                                if (dist < 5.5) {
                                    rlt->relation_graph[ul->users[k].devices[i]->uid * ul->user_count + ul->users[l].devices[j]->uid] ++;
                                    rlt->relation_graph[ul->users[l].devices[j]->uid * ul->user_count + ul->users[k].devices[i]->uid] ++;
                                    i = j = dl->device_count;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        switch (interval) {
            case YEAR:
                current_date += 31556926;
                break;
                
            case MONTH:
                current_date += 2629743;
                break;
                
            case DAY:
                current_date += 86400;
                break;
                
            case HOUR:
                current_date += 3600;
                break;
                
            default:
                break;
        }
    }
    
    return rlt;
}

void USR_print_users_stats(USR_user_list *ul) {
    printf("Total users : %ld\n", ul->user_count);
    printf("DEVICES MOBILE FIXE UNKNOWN   UID                                                         REAL-UID\n");
    for (long i=0; i<ul->user_count; i++) {
        printf("%7ld %6ld %4ld %7ld %5ld %s\n", ul->users[i].device_count, ul->users[i].stats[MOBILE], ul->users[i].stats[STATIC], ul->users[i].stats[UNKNOWN], i, ul->users[i].real_uid);
    }
}
