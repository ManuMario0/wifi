set(INC
    .
    ../csv
)

set(SRC
    intern/device.c
    
    intern/device.h
)

set(LIB
    mem_alloc
    kernel
)

add_module(device "${SRC}" "${INC}" "${LIB}")
