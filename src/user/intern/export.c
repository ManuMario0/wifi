//
//  export.c
//  empty
//
//  Created by Emmanuel Mera on 20/06/2023.
//

#include <string.h>

#include "USR_common.h"
#include "MEM_alloc.h"

#include "export.h"
#include "schedule.h"

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

static void record_device(FILE *f, DEVICE_device_list *dl, DEVICE_ap_list *apl, Device *d);

/* ---------------------------- */
/*  implementation              */
/* ---------------------------- */

/*
 File organisation :
 UID:....
 Devices:.....
 Mobile:...
 Static:....
 Schedule:begin
 Start:...;End:...;Activity:...;Device:....;
 ....
 end
 Device:begin
 MAC:
 Type:
 AP:begin
 UID:....;Freq:....;
 ....
 end
 end
 */

void USR_export_user_data(DEVICE_device_list *dl, DEVICE_ap_list *apl, USR_user *usr, char dir[]) {
    char path[2024];
    memcpy(path, dir, strlen(dir));
    memcpy(&path[strlen(dir)], usr->real_uid, strlen(usr->real_uid));
    memcpy(&path[strlen(dir)+strlen(usr->real_uid)], ".usr", strlen(".usr")+1);
    
    FILE *f = fopen(path, "w");
    
    fprintf(f, "UID:%s\n", usr->real_uid);
    fprintf(f, "Devices:%ld\n", usr->device_count);
    fprintf(f, "Mobile:%ld\n", usr->stats[MOBILE]);
    fprintf(f, "Static:%ld\n", usr->stats[STATIC]);
    
    for (int i=0; i<usr->device_count; i++) {
        Device *d = usr->devices[i];
        record_device(f, dl, apl, d);
    }
    
    fclose(f);
}

void USR_export_global_data(DEVICE_device_list *dl, DEVICE_ap_list *apl, char dir[]) {
    char path[2024];
    memcpy(path, dir, strlen(dir));
    memcpy(&path[strlen(dir)], "global.usr", strlen("global.usr")+1);
    
    FILE *f = fopen(path, "w");
    
    fprintf(f, "UID:Global\n");
    fprintf(f, "Devices:%ld\n", dl->effective_device_count);
    fprintf(f, "Mobile:%ld\n", dl->total_mobile);
    fprintf(f, "Static:%ld\n", dl->total_fixe);
    
    for (int i=0; i<dl->device_count; i++) {
        Device *d = &dl->devices[i];
        if (d->logs_count > 0) {
            record_device(f, dl, apl, d);
        }
    }
    
    fclose(f);
}

/* ---------------------------- */
/*  local functions             */
/* ---------------------------- */

void record_device(FILE *f, DEVICE_device_list *dl, DEVICE_ap_list *apl, Device *d) {
    CSV_cell *row = DEVICE_get_row(d, 0);
    char *mac = CSV_reverse_id(d->local_csv, MAC, row[MAC].l);
    
    fprintf(f, "Device:begin\n");
    fprintf(f, "MAC:%s\n", mac);
    switch (d->type) {
        case MOBILE:
            fprintf(f, "Type:MOBILE\n");
            break;
            
        case STATIC:
            fprintf(f, "Type:STATIC\n");
            break;
            
        case UNKNOWN:
            fprintf(f, "Type:UNKNOWN\n");
            break;
            
        default:
            break;
    }
    
    fprintf(f, "Schedule:begin\n");
    
    USR_schedule *schedule = USR_produce_device_schedule(dl, d);
    
    USR_event *e = schedule->events;
    for (long i=0; i<schedule->event_count; i++) {
        switch (e->type) {
            case MOVING:
                fprintf(f, "Start:%ld;End:%ld;Type:MOVING\n", e->start, e->end);
                break;
                
            case STOP:
                fprintf(f, "Start:%ld;End:%ld;Type:STOP\n", e->start, e->end);
                break;
                
            default:
                break;
        }
        
        e = e->next;
    }
    USR_destroy_schedule(schedule);
    
    fprintf(f, "end\n");
    
    fprintf(f, "AP:begin\n");
    
    int *ap_stats = MEM_calloc_array(sizeof(int), apl->ap_count, __func__);
    
    time_t start_data = d->start_time;
    start_data += 86400;
    char *ap_set = MEM_calloc(apl->ap_count, __func__);
    for (long j=0; j<d->logs_count; j++) {
        CSV_cell *current_row = DEVICE_get_row(d, j);
        if (current_row[TIMESTAMP].l > start_data) {
            start_data += 86400;
            memset(ap_set, 0, apl->ap_count);
        }
        char *ap_mac = CSV_reverse_id(d->local_csv, APMAC, current_row[APMAC].l);
        if (ap_mac && ap_mac[0] != '\0') {
            long ap = (long)KER_hash_find(apl->mac_addr, ap_mac, (int)strlen(ap_mac));
            if (!ap_set[ap-1])
                ap_stats[ap-1]++;
            ap_set[ap-1] = 1;
        }
    }
    
    for (long i=0; i<apl->ap_count; i++) {
        fprintf(f, "UID:%s;Freq:%d\n", apl->ap[i].name, ap_stats[i]);
    }
    
    MEM_free(ap_stats);
    MEM_free(ap_set);
    
    fprintf(f, "end\n");
    fprintf(f, "end\n");
}
