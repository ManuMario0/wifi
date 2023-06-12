set(INC
    .
    ../memory
)

set(SRC
    intern/hash_table.c
    intern/xxhash.c
    intern/stack.c

    KER_table.h
    KER_stack.h
    intern/xxhash.h
    intern/stack.h
)

set(LIB
    mem_alloc
)

add_module(kernel "${SRC}" "${INC}" "${LIB}")
