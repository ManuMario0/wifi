//
//  fingerprint.c
//  empty
//
//  Created by Emmanuel Mera on 05/07/2023.
//

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "DEVICE_common.h"
#include "MEM_alloc.h"
#include "KER_fft.h"
#include "CSV_common.h"

#include "fingerprint.h"

#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static long find_floor_index(DEV_device *d, time_t ts);
static long get_prefered_ap(DEV_ap_list *apl, DEV_device *d, time_t start, time_t end);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

/*
 Currently on the all period of presence but not pertinent I think ....
 In the future : per week or per year fp, or even by semesters, need to look more into it
 */
DEV_fp *DEV_get_fp(DEV_device_list *dl, DEV_ap_list *apl, DEV_device *d, time_t start, time_t end) {
    unsigned long period = end-start;
    long stop_signal = (long)KER_hash_find(d->local_csv->types[STATUS_TYPE].tbl, "Stop", 4);
    
    DEV_fp *fp = MEM_calloc(sizeof(DEV_fp), __func__);
    long ap = get_prefered_ap(apl, d, start, end);
    char *mac = CSV_reverse_id(d->local_csv, APMAC, ap);
    fp->prefered_AP = (long)KER_hash_find(apl->mac_addr, mac, (int)strlen(mac));
    
    ap = 1;
    
    float *ap_presence = MEM_calloc_array(sizeof(float), period, __func__);
    long start_index = find_floor_index(d, start);
    for (long i=start_index; i<d->logs_count; i++) {
        CSV_cell *past_row = DEV_get_row(d, i-1);
        CSV_cell *next_row = DEV_get_row(d, i);
        if (past_row[TIMESTAMP].l > end) break;
        if (past_row[STATUS_TYPE].l == stop_signal) {
            for (long j=MAX(past_row[TIMESTAMP].l, start); j<MIN(next_row[TIMESTAMP].l, end); j++) {
                ap_presence[j-start] = DEV_get_AP_distance(dl, ap, past_row[APMAC].l);
//                ap_presence[j-start] = past_row[APMAC].l;
//                ap_presence[j-start] = 1;
            }
        }
    }
    
    KER_static_rfft(ap_presence, period);
    
//    float mag = 0.;
//    long index = 0, current_threashold = 0;
//    for (long i=0; i<period; i++) {
//        if (i > fft_threashold[current_threashold]) {
//            fp->fingerprint[current_threashold] = index;
//            current_threashold++;
//            mag = ap_presence[i];
//            index = i;
//        }
//        if (ap_presence[i] > mag) {
//            mag = ap_presence[i];
//            index = i;
//        }
//    }
    
    memcpy(fp->fingerprint, ap_presence, FFT_FP*sizeof(float));
//    for (int i=0; i<20000; i++) {
//        printf("%3.3f ", ap_presence[i]);
//    }
//    printf("\n");
    MEM_free(ap_presence);
    
    return fp;
}

float DEV_compare_fp(DEV_fp *fp1, DEV_fp *fp2) {
    float dist = 0.;
    for (int i=0; i<FFT_FP; i++) {
        dist += fabsf(fp1->fingerprint[i] - fp2->fingerprint[i]);//*sqrtf((float)(i+1));
    }
    
    return dist;
}

void DEV_print_fp(DEV_fp *fp) {
    printf("Fingerprint :\n");
    printf("Favorite AP : %ld\n", fp->prefered_AP);
    for (long i=0; i<FFT_FP; i++) {
        printf("%2.3f, ", fp->fingerprint[i]);
//        printf("%6ld ", fp->fingerprint[i]);
    }
    printf("\n");
}

void DEV_export_device_fp(DEV_device_list *dl, DEV_ap_list *apl, DEV_device *d) {
    
}

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

long find_floor_index(DEV_device *d, time_t ts) {
    long left = 0;
    long right = d->local_csv->row_count-1;
    CSV_cell *row;
    
    while (left < right) {
        long m = (right+left+1)/2;
        row =  DEV_get_row(d, m);
        if (row[TIMESTAMP].l < ts)
            left = m;
        else
            right = m-1;
    }
    
    row = DEV_get_row(d, left);
    if (row[TIMESTAMP].l > ts)
        return --left;
    return left;
}

long find_max_index(long *tbl, long len) {
    long max = tbl[0];
    long index = 0;
    for (long i=0; i<len; i++) {
        if (tbl[i] > max) {
            max = tbl[i];
            index = i;
        }
    }
    return index;
}

long get_prefered_ap(DEV_ap_list *apl, DEV_device *d, time_t start, time_t end) {
    long stop_signal = (long)KER_hash_find(d->local_csv->types[STATUS_TYPE].tbl, "Stop", 4);
    
    long *timing = MEM_calloc_array(sizeof(long), apl->ap_count+1, __func__);
    
    long start_index = find_floor_index(d, start);
    for (long i=start_index; i<d->logs_count; i++) {
        CSV_cell *past_row = DEV_get_row(d, i-1);
        CSV_cell *next_row = DEV_get_row(d, i);
        if (past_row[TIMESTAMP].l > end) break;
        if (past_row[STATUS_TYPE].l != stop_signal) {
            timing[past_row[APMAC].l] += MIN(next_row[TIMESTAMP].l, end) - MAX(past_row[TIMESTAMP].l, start);
        }
    }
    
    long ap = find_max_index(timing, apl->ap_count+1);
    MEM_free(timing);
    
    return ap;
}
