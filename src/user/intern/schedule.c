//
//  schedule.c
//  empty
//
//  Created by Emmanuel Mera on 14/06/2023.
//

#include <limits.h>

#include "schedule.h"

#include "MEM_alloc.h"
#include "USR_common.h"

#define MIN(a, b) (a < b) ? (a) : (b)
#define MAX(a, b) (a > b) ? (a) : (b)

/* ---------------------------- */
/*  local structures            */
/* ---------------------------- */

typedef struct {
    float *speed;
    long speed_count;
    long *timestamp;
    long *AP;
} Speed;

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static USR_schedule *prepare_schedule(DEVICE_device_list *dl, USR_user *usr);
static void update_speed(DEVICE_device_list *dl,
                         USR_user *usr,
                         long *indexes,
                         time_t start,
                         float *speed,
                         long *index);
static float look_ahead(DEVICE_device_list *dl,
                        USR_user *usr,
                        long *indexes,
                        long index,
                        int distance);
static Speed *compute_speeds(DEVICE_device_list *dl, USR_user *usr);
static void compute_speed(DEVICE_device_list *dl,
                          USR_user *usr,
                          long *indexes,
                          time_t start,
                          float *speed,
                          long *index);
static void set_speed(Speed *s,
                      long *current_size,
                      long *current_pos,
                      float speed,
                      long timestamp,
                      long AP);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

USR_schedule *USR_get_user_schedule(DEVICE_device_list *dl, USR_user *usr) {
    compute_speeds(dl, usr);
    return prepare_schedule(dl, usr);
}

void USR_print_schedule(USR_schedule *schedule) {
    printf("Number of events : %ld\n", schedule->event_count);
    printf("   TYPE                 START                   END\n");
    
    USR_event *e = schedule->events;
    for (int i=0; i<schedule->event_count; i++) {
        switch (e->type) {
            case STOP:
                printf("   STOP   ");
                break;
                
            case MOVING:
                printf(" MOVING   ");
                break;
                
            default:
                break;
        }
        
        char buffer[2048];
        strftime(buffer, 2048, "%F %T", localtime(&e->start));
        printf("%s   ", buffer);
        
        strftime(buffer, 2048, "%F %T", localtime(&e->end));
        printf("%s\n", buffer);
        
        e = e->next;
    }
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
        row = DEVICE_get_row(d, m);
        if (row[TIMESTAMP].l < ts)
            left = m;
        else if (row[TIMESTAMP].l > ts)
            right = m-1;
        else
            return m;
    }
    
    row = DEVICE_get_row(d, left);
    if (row[TIMESTAMP].l > ts)
        return --left;
    return left;
}

void update_speed(DEVICE_device_list *dl,
                  USR_user *usr,
                  long *indexes,
                  time_t start,
                  float *speed,
                  long *index) {
    for (long i=0; i<usr->device_count; i++) {
        indexes[i] = find_floor_index(usr->devices[i], start);
        if (usr->devices[i]->type == MOBILE
            && indexes[i] >= 0
            && indexes[i]+1 < usr->devices[i]->logs_count) {
            CSV_cell *row = DEVICE_get_row(usr->devices[i], indexes[i]);
            CSV_cell *next_row = DEVICE_get_row(usr->devices[i], indexes[i]+1);
            
            float dist = DEVICE_get_AP_distance(dl, row[APMAC].l, next_row[APMAC].l);
            long time = next_row[TIMESTAMP].l - row[TIMESTAMP].l;
            
            if (time > 0) {
                *speed = MAX(*speed, dist / (float)time);
                if (dist / (float)time >= *speed)
                    *index = i;
            }
        }
    }
}

float look_ahead(DEVICE_device_list *dl,
                 USR_user *usr,
                 long *indexes,
                 long index,
                 int distance) {
    long *local_indexes = MEM_dupalloc(indexes, __func__);
    
    float speed = 0.;
    
    for (int i=0; i<distance; i++) {
        time_t start = DEVICE_get_row(usr->devices[index], indexes[index]+1)[TIMESTAMP].l;
        index = -1;
        
        update_speed(dl, usr, local_indexes, start, &speed, &index);
    }
    
    MEM_free(local_indexes);
    
    return speed;
}

/*
 * Right now, I'll use a simple classification of events based on an arbitrary speed threshold,
 * but ideally I'd like to have a more fine graine identification based on FFT and frequency analyses
 * Why not use a bit of deep learning to identify those behviors even though I think it'd be overkill considering
 * the current classification I'm using.
 */
USR_event *acquire_next_event(DEVICE_device_list *dl, USR_user *usr, time_t start) {
    USR_event *event = MEM_calloc(sizeof(USR_event), __func__);
    event->start = start;
    
    float speed = 0;
    long index = -1;
    
    long *indexes = MEM_malloc_array(sizeof(long), usr->device_count, __func__);
    
    update_speed(dl, usr, indexes, start, &speed, &index);
    
    if (speed > SPEED_THRESHOLD) {
        event->type = MOVING;
        while (speed > CORRECTED_SPEED_THRESHOLD) {
            start = DEVICE_get_row(usr->devices[index], indexes[index]+1)[TIMESTAMP].l;
            speed = 0.;
            
            update_speed(dl, usr, indexes, start, &speed, &index);
        }
    } else {
        event->type = STOP;
        while (speed <= SPEED_THRESHOLD || look_ahead(dl, usr, indexes, index, 1) <= SPEED_THRESHOLD) {
            if (index == -1)
                break;
            
            start = DEVICE_get_row(usr->devices[index], indexes[index]+1)[TIMESTAMP].l;
            speed = 0.;
            index = -1;
            
            update_speed(dl, usr, indexes, start, &speed, &index);
        }
    }
    
    event->end = start;
    
    MEM_free(indexes);
    
    return event;
}

USR_schedule *prepare_schedule(DEVICE_device_list *dl, USR_user *usr) {
    USR_schedule *schedule = MEM_calloc(sizeof(USR_schedule), __func__);
    
    schedule->uid = usr->uid;
    
    time_t start = LONG_MAX;
    time_t end = 0;
    for (int i=0; i<usr->device_count; i++) {
        start = MIN(start, usr->devices[i]->start_time);
        end = MAX(end, usr->devices[i]->end_time);
    }
    
    while (start < end) {
        USR_event *next_event = acquire_next_event(dl, usr, start);
        
        if (start == next_event->end) {
            MEM_free(next_event);
            break;
        }
        
        next_event->next = schedule->events;
        
        if (schedule->events) {
            next_event->prev = schedule->events->prev;
            
            schedule->events->prev->next = next_event;
            schedule->events->prev = next_event;
        } else {
            next_event->prev = next_event->next = next_event;
            schedule->events = next_event;
        }
        
        schedule->event_count++;
        
        start = next_event->end;
    }
    
    return schedule;
}

void compute_speed(DEVICE_device_list *dl,
                   USR_user *usr,
                   long *indexes,
                   time_t start,
                   float *speed,
                   long *index) {
    for (long i=0; i<usr->device_count; i++) {
        if (usr->devices[i]->type == MOBILE
            && indexes[i] >= 0
            && indexes[i]+1 < usr->devices[i]->logs_count) {
            CSV_cell *row = DEVICE_get_row(usr->devices[i], indexes[i]);
            CSV_cell *next_row = DEVICE_get_row(usr->devices[i], indexes[i]+1);
            
            float dist = DEVICE_get_AP_distance(dl, row[APMAC].l, next_row[APMAC].l);
            long time = next_row[TIMESTAMP].l - row[TIMESTAMP].l;
            
            if (time > 0) {
                *speed = MAX(*speed, dist / (float)time);
                if (dist / (float)time >= *speed)
                    *index = i;
            } else {
                *speed = MAX(*speed, dist);
                if (dist >= *speed)
                    *index = i;
            }
        }
    }
}

void set_speed(Speed *s, long *current_size, long *current_pos, float speed, long timestamp, long AP) {
    if (*current_pos+1 < *current_size) {
        ++*current_pos;
        s->speed[*current_pos] = speed;
        s->timestamp[*current_pos] = timestamp;
        s->AP[*current_pos] = AP;
    } else {
        *current_size *= 2;
        s->speed = MEM_realloc(s->speed, sizeof(float)*(*current_size));
        s->timestamp = MEM_realloc(s->timestamp, sizeof(long)*(*current_size));
        s->AP = MEM_realloc(s->AP, sizeof(long)*(*current_size));
    }
}

Speed *compute_speeds(DEVICE_device_list *dl, USR_user *usr) {
    Speed *s = MEM_calloc(sizeof(Speed), __func__);
    long current_size = 1024;
    long current_pos = 0;
    s->speed = MEM_malloc_array(sizeof(float), current_size, __func__);
    s->timestamp = MEM_malloc_array(sizeof(long), current_size, __func__);
    s->AP = MEM_malloc_array(sizeof(long), current_size, __func__);
    
    long start = LONG_MAX, end = 0;
    float speed = 0;
    long index = -1;
    long *indexes = MEM_malloc_array(sizeof(long), usr->device_count, __func__);
    
    for (int i=0; i<usr->device_count; i++) {
        if (usr->devices[i]->type == MOBILE) {
            start = MIN(start, usr->devices[i]->start_time);
            end = MAX(end, usr->devices[i]->end_time);
        }
    }
    
    for (long i=0; i<usr->device_count; i++) {
        indexes[i] = find_floor_index(usr->devices[i], start);
    }
    
    while (start < end) {
        compute_speed(dl, usr, indexes, start, &speed, &index);
        set_speed(s, &current_size, &current_pos, speed, start, DEVICE_get_row(usr->devices[index], indexes[index])[APMAC].l);
        if (index != -1)
            indexes[index]++;
        start = DEVICE_get_row(usr->devices[index], indexes[index])[TIMESTAMP].l;
        
        for (int i=0; i<usr->device_count; i++) {
            if (i != index)
                indexes[i] = find_floor_index(usr->devices[i], start);
        }
        speed = 0;
        index = -1;
    }
    
    MEM_free(indexes);
    
    return s;
}

/*
 NOTES :
 
 - precompute speeds, might be usefull at some point
 - be aware of the disconnection time (maybe use a third identifier like HOME)
 - use the lookahead like a MM, so that we can eliminate the noise
    (we dont want any moving or home period of less than 10 secs)
 - clean up code (used this one as a test)
 - add AP classification during paring so that we might after that post process the data
    to identify working areas, eating areas and so on ...
 - later, I will be able to use the pin-point location of the AP but I'll have to build
    a corresponding database
 - check with ground truth whenever possible
 
 */
