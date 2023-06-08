//
//  device.h
//  empty
//
//  Created by Emmanuel Mera on 08/06/2023.
//

#ifndef device_h
#define device_h

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

#endif /* device_h */
