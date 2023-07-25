set(INC
    .
    ../memory
)

set(SRC
    intern/hash_table.c
    intern/xxhash.c
    intern/stack.c
    intern/fft.c
    intern/list.c

    KER_table.h
    KER_stack.h
    KER_list.h
    intern/xxhash.h
    intern/stack.h
    intern/fft.h
)

set(LIB
    mem_alloc
)

add_module(kernel "${SRC}" "${INC}" "${LIB}")
