set(INC
    .
    ../csv
    ../device
)

set(SRC
    intern/user.c
    intern/schedule.c
    intern/export.c

    USR_common.h
    intern/user.h
    intern/schedule.h
    intern/export.h
)

set(LIB
    mem_alloc
    kernel
)

add_module(user "${SRC}" "${INC}" "${LIB}")
