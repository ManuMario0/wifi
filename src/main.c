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
    
    FILE *fp = fopen("/Volumes/Emmanuel/stage[DELETE]/fix_database.csv", "r");
    
    int flags[] = {CSV_DATE, CSV_LONG, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_AGGLO, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_LONG, CSV_AGGLO, CSV_SKIP, CSV_SKIP, CSV_AGGLO};
    
    CSV_file *csv = CSV_parse(fp, flags, sizeof(flags));
    
    Device **devices = MEM_malloc_array(csv->types[MAC].item_count, sizeof(Device*), __func__);
    long significant_data = 0;
    
    DEVICE_device_list *dl = DEVICE_create_device_list(csv);
    
    printf("Weird AP : %ld\n", (long)KER_hash_find(dl->devices->local_csv->types[APMAC].tbl, "\0", 0));
    
    USR_user_list *ul = USR_create_user_list(dl);
    
    for (long i=0; i<ul->user_count; i++) {
        //printf("User %ld : %ld devices\n", i, ul->users[i].device_count);
        significant_data += ul->users[i].device_count;
    }
    
    printf("Total device count : %ld\n", significant_data);
    
    
    
    CSV_date start = {0};
    start.year = 2022;
    start.month = 11;
    start.day = 2;
    
    CSV_date period_length = {0};
    period_length.day = 1;
    
    CSV_date record_start = {0};
    record_start.hour = 13;
    
    CSV_date record_end = {0};
    record_end.hour = 14;
    
    USR_relation *rel_sept = USR_create_user_relation_graph(ul, dl, start, period_length, DAY, record_start, record_end, MOBILE);
    
    for (int i=0; i<ul->user_count; i++) {
        for (int j=i+1; j<ul->user_count; j++) {
            if (rel_sept->relation_graph[i*ul->user_count+j] > 0)
                printf("%d ; %d :: %ld\n", i, j, rel_sept->relation_graph[i*ul->user_count+j]);
        }
    }
    
    MEM_print_memstats();
    
    return 0;
}
