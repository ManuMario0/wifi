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
} DEV_ap;

typedef struct {
    long            ap_count;
    DEV_ap *        ap;
    KER_hashTable * mac_addr;
    KER_hashTable * translate;
    
    long            stats[5];
} DEV_ap_list;

/*
 Get the AP data (names, MAD addr, etc ....) from the database
 */
extern DEV_ap_list *DEV_acquire_access_points(char filename[]);

/*
 Print general statistics on the AP
 */
extern void DEV_print_ap_stats(DEV_ap_list *apl);

// structure of the device database
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
} DEV_device;

typedef struct {
    DEV_device *devices;
    long device_count;
    long effective_device_count;
    long AP_count;
    long skip_logs;
    
    CSV_file *csv;
    
    long total_fixe;
    long total_mobile;
    long total_unknown;
    
    float *AP_graph;
} DEV_device_list;

/*
 Acquire the device list (and then some)
 */
extern DEV_device_list *DEV_create_device_list(char filename[]);

/*
 Estimate the physical distance between APs by averaging the time
 between two logs of different AP
 */
extern float *DEV_compute_AP_graph(CSV_file *f);

/*
 Compute the average distance between two users using the AP graph
 */
extern float DEV_proximity(DEV_device *d1, DEV_device *d2, time_t start_ts, time_t end_ts, DEV_device_list *dl);

/*
 Get theh distance between AP1 and AP2
 */
extern float DEV_get_AP_distance(DEV_device_list *dl, long AP1, long AP2);

/*
 Print general statistics on the device database
 */
extern void DEV_print_devices_stats(DEV_device_list *dl);

/*
 Store the AP graph to be further processed with other software
 */
extern void DEV_store_graph(DEV_device_list *dl, char *filename);

/*
 Get a log from the device
 */
extern CSV_cell *DEV_get_row(DEV_device *d, long index);



/*
 Some fancy try : NOT WORKING ! DO NOT TOUCH !
 */

//#define LEN_FP 10000
static int fft_threashold[] = {1, 10, 50, 100, 250, 500, 1000, 5000, 10000, 20000, 50000};
#define FFT_FP 100

typedef struct {
    long    prefered_AP;
    float   fingerprint[FFT_FP];
} DEV_fp;

extern DEV_fp *DEV_get_fp(DEV_device_list *dl, DEV_ap_list *apl, DEV_device *d, time_t start, time_t end);
extern float DEV_compare_fp(DEV_fp *fp1, DEV_fp *fp2);
extern void DEV_print_fp(DEV_fp *fp);

#endif /* DEVICE_common_h */
