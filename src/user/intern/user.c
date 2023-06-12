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

USR_user_list *USR_create_user_list(DEVICE_device_list *dl) {
    USR_user_list *ul = MEM_malloc(sizeof(USR_user_list), __func__);
    ul->user_count = dl->devices->local_csv->types[UID].item_count;
    ul->users = MEM_calloc_array(ul->user_count, sizeof(USR_user), __func__);
    
    for (int i=0; i<dl->device_count; i++) {
        Device *d = &dl->devices[i];
        if (d->logs_count > 50 && d->end_time - d->start_time > 2629743) {
            ul->users[d->uid-1].device_count ++;
            ul->users[d->uid-1].stats[d->type] ++;
        }
    }
    
    for (int i=0; i<ul->user_count; i++) {
        ul->users[i].devices = MEM_malloc_array(ul->users[i].device_count, sizeof(Device*), __func__);
        ul->users[i].device_count = 0;
    }
    
    for (int i=0; i<dl->device_count; i++) {
        Device *d = &dl->devices[i];
        if (d->logs_count > 50 && d->end_time - d->start_time > 2629743) {
            ul->users[d->uid-1].devices[ul->users[d->uid-1].device_count] = d;
            ul->users[d->uid-1].device_count ++;
        }
    }
    
    return ul;
}

USR_relation *USR_create_user_relation_graph(USR_user_list *    ul,
                                             DEVICE_device_list *dl,
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
            end_tm.tm_mon = rlt->start.month + period_length.month;
            
        case YEAR:
            end_tm.tm_year = rlt->start.year + period_length.year;
            
        default:
            break;
    }
    
    time_t end = mktime(&end_tm);
    
    
    struct tm current_date_tm = {0};
    current_date_tm.tm_year = rlt->start.year;
    current_date_tm.tm_mon = rlt->start.month;
    current_date_tm.tm_mday = rlt->start.day;
    current_date_tm.tm_hour = rlt->start.hour;
    current_date_tm.tm_isdst = 1;
    
    time_t current_date = mktime(&current_date_tm);
    
    memset(&current_date_tm, 0, sizeof(struct tm));
    current_date_tm.tm_year = rlt->start.year;
    current_date_tm.tm_mon = rlt->start.month;
    current_date_tm.tm_mday = rlt->start.day;
    current_date_tm.tm_hour = rlt->start.hour;
    
    while (current_date < end) {
        for (long k=0; k<ul->user_count; k++) {
            for (long l=k+1; l<ul->user_count; l++) {
                for (long i=0; i<ul->users[k].device_count; i++) {
                    if (ul->users[k].devices[i]->type & filter) {
                        for (long j=0; j<ul->users[l].device_count; j++) {
                            if (ul->users[l].devices[j]->type & filter) {
                                CSV_date start_local, end_local;
                                start_local.year = current_date_tm.tm_year;
                                start_local.month = current_date_tm.tm_mon;
                                start_local.day = current_date_tm.tm_mday;
                                start_local.hour = current_date_tm.tm_hour;
                                start_local.min = start_local.sec = 0;
                                
                                end_local.year = current_date_tm.tm_year;
                                end_local.month = current_date_tm.tm_mon;
                                end_local.day = current_date_tm.tm_mday;
                                end_local.hour = current_date_tm.tm_hour;
                                end_local.min = end_local.sec = 0;
                                switch (interval) {
                                    case YEAR:
                                        start_local.month += record_start.month;
                                        end_local.month += record_end.month;
                                        
                                    case MONTH:
                                        start_local.day += record_start.day;
                                        end_local.day += record_end.day;
                                        
                                    case DAY:
                                        start_local.hour += record_start.hour;
                                        end_local.hour += record_end.hour;
                                        
                                    case HOUR:
                                        start_local.min += record_start.min;
                                        end_local.min += record_end.min;
                                        start_local.sec += record_start.sec;
                                        end_local.sec += record_end.sec;
                                        
                                    default:
                                        break;
                                }
                                
                                float dist = DEVICE_proximity(ul->users[k].devices[i], ul->users[l].devices[j], start_local, end_local, dl);
                                
                                if (dist < 6.) {
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
                current_date_tm.tm_year++;
                current_date += 31556926;
                break;
                
            case MONTH:
                current_date_tm.tm_mon++;
                current_date += 2629743;
                break;
                
            case DAY:
                current_date_tm.tm_mday++;
                current_date += 86400;
                break;
                
            case HOUR:
                current_date_tm.tm_hour++;
                current_date += 3600;
                break;
                
            default:
                break;
        }
    }
    
    return rlt;
}
