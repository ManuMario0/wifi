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
#include "USR_common.h"

int main(void) {
    //MEM_use_secured_allocator();
    
    long significant_data = 0;
    
    DEVICE_ap_list *apl = DEVICE_acquire_access_points("/Volumes/Emmanuel/stage[DELETE]/devices.csv");
    DEVICE_device_list *dl = DEVICE_create_device_list("/Volumes/Emmanuel/stage[DELETE]/fix_database.csv");
    
    DEVICE_store_graph(dl, "/Users/emmanuel/Documents/developement/wifi/test.txt");
    
    DEVICE_print_devices_stats(dl);
    
    USR_user_list *ul = USR_create_user_list(dl);
    USR_print_users_stats(ul);
    
    for (long i=0; i<ul->user_count; i++) {
        significant_data += ul->users[i].device_count;
    }
    
    printf("Total device count : %ld / %ld\n", significant_data, dl->device_count);
    
    CSV_date start = {0};
    start.year = 2023;
    start.month = 1;
    start.day = 1;
    
    CSV_date period_length = {0};
    period_length.month = 4;
    
    CSV_date record_start = {0};
    record_start.hour = 9;
    
    CSV_date record_end = {0};
    record_end.hour = 13;
    
    USR_relation *rel_sept = USR_create_user_relation_graph(ul, dl, start, period_length, DAY, record_start, record_end, MOBILE);
    
    for (int i=0; i<ul->user_count; i++) {
        for (int j=i+1; j<ul->user_count; j++) {
            if (rel_sept->relation_graph[i*ul->user_count+j] > 1)
                printf("%d -- %d [len=%f] ;\n", i, j, 30./(float)rel_sept->relation_graph[i*ul->user_count+j]);
        }
    }
    
    USR_schedule *schedule = USR_get_user_schedule(dl, &ul->users[10]);
    USR_print_schedule(schedule);
    printf("\n\n\n\n");
    
    schedule = USR_get_user_schedule(dl, &ul->users[14]);
    USR_print_schedule(schedule);
    
    MEM_print_memstats();
    
    return 0;
}
