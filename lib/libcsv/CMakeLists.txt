set(INC
    .
)

set(SRC
    libcsv.c
    csv.h
)

set(LIB
)

add_module(libcsv "${SRC}" "${INC}" "${LIB}")
