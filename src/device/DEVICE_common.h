//
//  DEVICE_common.h
//  empty
//
//  Created by Emmanuel Mera on 08/06/2023.
//

#ifndef DEVICE_common_h
#define DEVICE_common_h

enum {
    MOBILE,
    FIXE,
    UNKNOWN
};

typedef struct {
    int     type;
    
    long    logs_count;
    
    float   average_changes_per_day;
    float   average_AP_per_day;
} Device;

extern Device *create_device(int mac, CSV_file *f);

#endif /* DEVICE_common_h */
