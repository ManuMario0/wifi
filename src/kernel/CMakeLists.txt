set(INC
    .
    ../memory
)

set(SRC
    intern/hash_table.c
    intern/xxhash.c

    KER_table.h
    intern/xxhash.h
)

set(LIB
    mem_alloc
)

add_module(kernel "${SRC}" "${INC}" "${LIB}")
