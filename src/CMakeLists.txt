# -----------------------------------
# - main sources and dependencies   -
# -----------------------------------

target_sources(wifi PUBLIC main.c)
set_target_properties(wifi PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})

# -----------------------------------
# - subdirectories                  -
# -----------------------------------

add_subdirectory("memory")
add_subdirectory("kernel")
add_subdirectory("csv")
add_subdirectory("device")
add_subdirectory("user")
