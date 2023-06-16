//
//  access_point.c
//  empty
//
//  Created by Emmanuel Mera on 16/06/2023.
//

#include "access_point.h"

#include "MEM_alloc.h"

#include "CSV_common.h"

DEVICE_ap_list *DEVICE_acquire_access_points(char filename[]) {
    int flags[] = {CSV_AGGLO, CSV_SKIP, CSV_AGGLO, CSV_SKIP, CSV_SKIP, CSV_AGGLO, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP, CSV_SKIP};
    CSV_file *f = CSV_parse(filename, flags, sizeof(flags));
    
    DEVICE_ap_list *apl = MEM_calloc(sizeof(DEVICE_ap_list), __func__);
    
    apl->ap_count = f->row_count;
    apl->ap = MEM_calloc_array(sizeof(DEVICE_ap), apl->ap_count, __func__);
    
    for (long i=0; i<apl->ap_count; i++) {
        CSV_cell *row = CSV_get_row(f, i);
        
        apl->ap[i].apid = i;
        apl->ap[i].mac = CSV_reverse_id(f, AP_MAC, row[AP_MAC].l);
        apl->ap[i].name = CSV_reverse_id(f, AP_NAME, row[AP_NAME].l);
        apl->ap[i].map_location = CSV_reverse_id(f, AP_MAPLOC, row[AP_MAPLOC].l);
        switch (apl->ap[i].name[0]) {
            case 'M':
                apl->ap[i].location = MADRID;
                break;
                
            case 'A':
                apl->ap[i].location = GETAFE;
                break;
                
            case 'B':
                apl->ap[i].location = LEGANES;
                break;
                
            case 'C':
                apl->ap[i].location = COLMERAJERO;
                break;
                
            default:
                break;
        }
    }
    
    return apl;
}
