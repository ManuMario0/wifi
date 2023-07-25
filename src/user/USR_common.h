//
//  USR_common.h
//  wifi
//
//  Created by Emmanuel Mera on 09/06/2023.
//

#ifndef USR_common_h
#define USR_common_h

#include <time.h>

#include "DEVICE_common.h"

enum {
    HOUR,
    DAY,
    MONTH,
    YEAR
};

typedef struct {
    CSV_date    start;
    CSV_date    period_length;
    int         interval;
    CSV_date    record_start;
    CSV_date    record_end;
    
    long *      relation_graph;
} USR_relation;

typedef struct {
    long        uid;
    char *      real_uid;
    
    long        device_count;
    long        stats[5];
    DEV_device **   devices;
} USR_user;

typedef struct {
    long            user_count;
    USR_user *      users;
} USR_user_list;

/*
 Create users
 */
extern USR_user_list *USR_create_user_list(DEV_device_list *dl);
extern USR_relation *USR_create_user_relation_graph(USR_user_list *    ul,
                                                    DEV_device_list *dl,
                                                    CSV_date           start,
                                                    CSV_date           period_length,
                                                    int                interval,
                                                    CSV_date           record_start,
                                                    CSV_date           record_end,
                                                    int                filter);

/*
 Print users statistics
 */
extern void USR_print_users_stats(USR_user_list *ul);

typedef struct USR_event {
    int     type;
    long    AP;
    
    time_t  start;
    time_t  end;
    
    struct USR_event    *next, *prev;
} USR_event;

typedef struct {
    USR_event * events;
    
    long        event_count;
    long        uid;
} USR_schedule;

/*
 Compute user schedule (not working properly)
 */
extern USR_schedule *USR_get_user_schedule(DEV_device_list *dl, USR_user *usr);

/*
 Print user schedule
 */
extern void USR_print_schedule(DEV_ap_list *apl, USR_schedule *schedule);

/*
 Export user data (format developed in inter/export.c)
 */
extern void USR_export_user_data(DEV_device_list *dl, DEV_ap_list *apl, USR_user *usr, char dir[]);

/*
 Produce device schedule based on the approximate speed of the user
 */
extern USR_schedule *USR_produce_device_schedule(DEV_device_list *dl, DEV_ap_list *apl, DEV_device *d);

/*
 Produce device schedule based on the holding time of AP
 (if a device holds an AP for more than 10 minutes, we say that the device is static)
 */
extern USR_schedule *USR_produce_device_schedule_v2(DEV_device_list *dl, DEV_ap_list *apl, DEV_device *d);

/*
 Destroy schedule
 */
extern void USR_destroy_schedule(USR_schedule *schedule);

/*
 Export the data of all the devices in a 'global' user file format
 */
extern void USR_export_global_data(DEV_device_list *dl, DEV_ap_list *apl, char dir[]);

#endif /* USR_common_h */
