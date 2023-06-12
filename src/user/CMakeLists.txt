set(INC
    .
    ../csv
    ../device
)

set(SRC
    intern/user.c

    USR_common.h
    intern/user.h
)

set(LIB
    mem_alloc
    kernel
)

add_module(user "${SRC}" "${INC}" "${LIB}")
