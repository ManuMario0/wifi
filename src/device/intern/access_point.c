//
//  access_point.c
//  empty
//
//  Created by Emmanuel Mera on 16/06/2023.
//

#include <string.h>

#include "access_point.h"

#include "MEM_alloc.h"

#include "CSV_common.h"

DEV_ap_list *DEV_acquire_access_points(char filename[]) {
    int flags[] = {CSV_AGGLO, CSV_SKIP, CSV_AGGLO, CSV_SKIP, CSV_SKIP, CSV_AGGLO, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP};
    CSV_file *f = CSV_parse(filename, flags, sizeof(flags));
    
    DEV_ap_list *apl = MEM_calloc(sizeof(DEV_ap_list), __func__);
    
    apl->ap_count = f->row_count;
    apl->mac_addr = f->types[AP_MAC].tbl;
    apl->translate = KER_hash_create_table(2048);
    apl->ap = MEM_calloc_array(sizeof(DEV_ap), apl->ap_count, __func__);
    
    for (long i=0; i<apl->ap_count; i++) {
        CSV_cell *row = CSV_get_row(f, i);
        
        apl->ap[i].apid = i;
        apl->ap[i].mac = CSV_reverse_id(f, AP_MAC, row[AP_MAC].l);
        apl->ap[i].name = CSV_reverse_id(f, AP_NAME, row[AP_NAME].l);
        KER_hash_add(apl->translate, apl->ap[i].mac, strlen(apl->ap[i].mac), apl->ap[i].name);
        apl->ap[i].map_location = CSV_reverse_id(f, AP_MAPLOC, row[AP_MAPLOC].l);
        switch (apl->ap[i].name[0]) {
            case 'M':
                apl->ap[i].location = MADRID;
                apl->stats[MADRID]++;
                break;
                
            case 'A':
                apl->ap[i].location = GETAFE;
                apl->stats[GETAFE]++;
                break;
                
            case 'B':
                apl->ap[i].location = LEGANES;
                apl->stats[LEGANES]++;
                break;
                
            case 'C':
                apl->ap[i].location = COLMENAJERO;
                apl->stats[COLMENAJERO]++;
                break;
                
            default:
                break;
        }
    }
    
    return apl;
}

void DEV_print_ap_stats(DEV_ap_list *apl) {
    printf("Total AP : %ld\n", apl->ap_count);
    printf("Total AP at Madrid : %ld\n", apl->stats[MADRID]);
    printf("Total AP at Getafe : %ld\n", apl->stats[GETAFE]);
    printf("Total AP at Leganes : %ld\n", apl->stats[LEGANES]);
    printf("Total AP at Colmenajero : %ld\n", apl->stats[COLMENAJERO]);
}
