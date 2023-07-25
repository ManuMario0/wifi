set(INC
    ../../lib/freetype/include
    ../kernel
    ../memory
    ../../lib/
    .
)

set(SRC
    intern/init.c
    intern/command_buffer.c
    intern/renderer.c
    intern/model_loading.c
    intern/buffer.c
    intern/model.c
    intern/wavefront.c
    intern/object.c
    intern/camera.c
    intern/ui.c
    intern/texture.c
    intern/descriptor_set.c
    intern/device_alloc.c
    
    RND_window.h
    intern/init.h
    intern/command_buffer.h
    intern/renderer.h
    intern/model_loading.h
    intern/buffer.h
    intern/model.h
    intern/wavefront.h
    intern/object.h
    intern/camera.h
    intern/ui.h
    intern/texture.h
    intern/descriptor_set.h
    intern/device_alloc.h
)

set(LIB
    mem_alloc
    kernel
)

add_module(renderer "${SRC}" "${INC}" "${LIB}")
