set(INC
    .
    ../csv
)

set(SRC
    intern/device.c
    intern/access_point.c
    intern/fingerprint.c
    
    DEVICE_common.h
    intern/device.h
    intern/access_point.h
    intern/fingerprint.h
)

set(LIB
    mem_alloc
    kernel
    csv
)

add_module(device "${SRC}" "${INC}" "${LIB}")
