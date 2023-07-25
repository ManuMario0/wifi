//
//  main.c
//  wifi
//
//  Created by Emmanuel Mera on 05/06/2023.
//

#include <stdio.h>

#include "MEM_alloc.h"
#include "DEVICE_common.h"
#include "USR_common.h"
//#include "VSL_common.h"

#define INPUT_AP_DATABASE ""
#define INPUT_DEVICE_DATABASE ""
#define USER_LOG_OUTPUT_DIR ""

int main(void) {
//    MEM_use_secured_allocator();
    
    DEV_ap_list *apl = DEV_acquire_access_points(INPUT_AP_DATABASE);
    DEV_device_list *dl = DEV_create_device_list(INPUT_DEVICE_DATABASE);
    
    DEV_print_devices_stats(dl);
    DEV_print_ap_stats(apl);
    
    USR_user_list *ul = USR_create_user_list(dl);
    USR_print_users_stats(ul);
    
    printf("Total device count : %ld / %ld\n", dl->effective_device_count, dl->device_count);
    
    USR_schedule *schedule = USR_produce_device_schedule_v2(dl, apl, ul->users[32].devices[0]);
    USR_print_schedule(apl, schedule);
    
    for (int i=0; i<ul->user_count; i++) {
        USR_export_user_data(dl, apl, &ul->users[i], USER_LOG_OUTPUT_DIR);
    }
    USR_export_global_data(dl, apl, USER_LOG_OUTPUT_DIR);
    
    MEM_print_memstats();
    
//    VSL_launchVisualization(ul, apl, dl);
//    VSL_launch_animation(ul, apl, dl);
    
    return 0;
}
