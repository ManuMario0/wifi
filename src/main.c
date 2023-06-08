//
//  main.c
//  wifi
//
//  Created by Emmanuel Mera on 05/06/2023.
//

#include <stdio.h>

#include "CSV_common.h"
#include "MEM_alloc.h"
#include "DEVICE_common.h"

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

int main(void) {
    //MEM_use_secured_allocator();
    
    FILE *fp = fopen("/Volumes/Emmanuel/stage[DELETE]/wifi_2022-23.csv", "r");
    
    int flags[] = {CSV_DATE, CSV_LONG, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_AGGLO, CSV_SKIP, CSV_SKIP, CSV_AGGLO};
    
    CSV_file *csv = CSV_parse(fp, flags, sizeof(flags));
    
    Device **devices = MEM_malloc_array(csv->types[MAC].item_count, sizeof(Device*), __func__);
    int significant_data = 0;
    
    for (int i=1; i<=csv->types[MAC].item_count; i++) {
        Device *d = create_device(i, csv);
        if (d->logs_count > 50) {
            printf("%d : %f ; %f ; %ld\n", i, d->average_AP_per_day, d->average_changes_per_day, d->logs_count);
            devices[significant_data] = d;
            significant_data ++;
        }
    }
    
    printf("Retained data : %d\n", significant_data);
    
    
    
    MEM_print_memstats();
    
    return 0;
}
