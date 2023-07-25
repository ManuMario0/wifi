set(INC
    .
    ../memory
    ../kernel
    ../renderer
    ../user
)

set(SRC
    intern/main_loop.c
    intern/animation.c
    
    VSL_common.h
    intern/main_loop.h
    intern/animation.h
)

set(LIB
    mem_alloc
    kernel
    renderer
    user
)

add_module(visualization "${SRC}" "${INC}" "${LIB}")
