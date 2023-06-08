set(INC
    .
)

set(SRC
    ./intern/alloc_secured_impl.c
    ./intern/alloc_light_impl.c
    ./intern/malloc.c
    
    ./MEM_alloc.h
    ./intern/alloc_secured_impl.h
    ./intern/alloc_light_impl.h
)

set(LIB
)

add_module(mem_alloc "${SRC}" "${INC}" "${LIB}")
