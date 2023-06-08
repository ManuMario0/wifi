set(INC
    .
    ../kernel
)

set(SRC
    intern/csv.c
    
    CSV_common.h
    intern/csv.h
)

set(LIB
    mem_alloc
    kernel
)

add_module(csv "${SRC}" "${INC}" "${LIB}")
