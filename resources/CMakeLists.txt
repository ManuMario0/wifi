# ------------------------
# - Shaders installation -
# ------------------------

set(RESOURCE_FILES
    shaders/vert.spv
    shaders/frag.spv
    shaders/texture_vert.spv
    shaders/texture_frag.spv
    
    models/sphere.3d
    models/sphere.mtl
    models/cube.3d
    models/cube.mtl
    models/link.3d
    models/link.mtl
)

add_resources("${RESOURCE_FILES}")
