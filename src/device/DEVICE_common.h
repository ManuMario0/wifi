//
//  DEVICE_common.h
//  empty
//
//  Created by Emmanuel Mera on 08/06/2023.
//

#ifndef DEVICE_common_h
#define DEVICE_common_h

#include <time.h>

#include "CSV_common.h"

typedef struct {
    long        apid;
    char *      mac;
    int         location;
    char *      name;
    char *      map_location;
} DEVICE_ap;

typedef struct {
    long        ap_count;
    DEVICE_ap * ap;
    KER_hashTable *mac_addr;
    
    long        stats[5];
} DEVICE_ap_list;

extern DEVICE_ap_list *DEVICE_acquire_access_points(char filename[]);
extern void DEVICE_print_ap_stats(DEVICE_ap_list *apl);

enum {
    DATE,
    TIMESTAMP,
    UID,
    MAC,
    STATUS_TYPE,
    USESSION_ID,
    SESSION_ID,
    OCTETS_OUT,
    PACKETS_OUT,
    GOS_OUT,
    PACKETS_IN,
    OCTETS_IN,
    GOS_IN,
    APMAC,
    UIP_ADDR
};

enum {
    MOBILE = 0x1,
    STATIC = 0x2,
    UNKNOWN = 0x4
};

typedef struct {
    int         mac;
    int         type;
    long        uid;
    char *      real_mac;
    
    CSV_file *  local_csv;
    
    long        logs_count;
    long        skiped_logs;
    
    time_t  start_time;
    time_t  end_time;
    
    float       average_changes_per_day;
    float       average_AP_per_day;
} Device;

typedef struct {
    Device *devices;
    long device_count;
    long effective_device_count;
    long AP_count;
    
    CSV_file *csv;
    
    long total_fixe;
    long total_mobile;
    long total_unknown;
    
    float *AP_graph;
} DEVICE_device_list;

extern DEVICE_device_list *DEVICE_create_device_list(char filename[]);
extern float *DEVICE_compute_AP_graph(CSV_file *f);
extern float DEVICE_proximity(Device *d1, Device *d2, time_t start_ts, time_t end_ts, DEVICE_device_list *dl);
extern float DEVICE_get_AP_distance(DEVICE_device_list *dl, long AP1, long AP2);
extern void DEVICE_print_devices_stats(DEVICE_device_list *dl);
extern void DEVICE_store_graph(DEVICE_device_list *dl, char *filename);
extern CSV_cell *DEVICE_get_row(Device *d, long index);

#endif /* DEVICE_common_h */
