# -----------------------------------
# - main sources and dependencies   -
# -----------------------------------

target_sources(wifi PUBLIC main.c)
set_target_properties(wifi PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})

# -----------------------------------
# - subdirectories                  -
# -----------------------------------

add_subdirectory("memory")
