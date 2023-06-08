# ---------------------------------
# - module managment              -
# ---------------------------------

function(add_module
    name
	sources
	includes
	lib)
    
    add_library(${name} ${sources})
    target_include_directories(${name} PUBLIC ${includes})
    target_link_libraries(${name} ${lib})
    target_link_libraries(wifi PUBLIC ${name})
    target_include_directories(wifi PUBLIC ${CMAKE_CURRENT_DIR})
endfunction()

function(add_resources
    files)

    target_sources(wifi PUBLIC ${files})
if(APPLE)
    foreach(element IN LISTS files)
        list(APPEND res_files ${CMAKE_CURRENT_SOURCE_DIR}/${element})
    endforeach()
    
    set_target_properties(wifi PROPERTIES
                        FRAMEWORK TRUE
                        MACOSX_BUNDLE TRUE
                        MACOSX_FRAMEWORK_IDENTIFIER org.cmake.synthesis
                        RESOURCE "${res_files}")
endif()
if(UNIX AND NOT APPLE)
    foreach(element IN LISTS files)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${element} ${CMAKE_BINARY_DIR}/resources COPYONLY)
    endforeach()
endif()
if(WIN32)
    foreach(element IN LISTS files)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${element} ${CMAKE_BINARY_DIR}/resources COPYONLY)
    endforeach()
endif()

endfunction()
