cmake_minimum_required(VERSION 3.10)

# -----------------------------------
# - windows file names issue        -
# -----------------------------------

file(TO_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR} CMAKE_CURRENT_SOURCE_DIR)
file(TO_CMAKE_PATH ${CMAKE_BINARY_DIR} CMAKE_BINARY_DIR)

# -----------------------------------
# - create project		            -
# -----------------------------------

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER gcc)

project(trading VERSION 0.0.1)
add_executable(wifi)
set(CMAKE_BUILD_TYPE Debug)

# -----------------------------------
# - set policy for relative paths   -
# -----------------------------------

cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)

# -----------------------------------
# - environment control		        -
# -----------------------------------

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please run \"cmake ..\" from the build directory. You may need to delete \"${CMAKE_SOURCE_DIR}/CMakeCache.txt\" first.")
endif()

# -----------------------------------
# - custom functions		        -
# -----------------------------------

include(cmake/macros.cmake)

# -----------------------------------
# - macOS specific issues           -
# -----------------------------------

if(APPLE)
    include_directories("/usr/local/include")
endif()

# -----------------------------------
# - Windows specific issues           -
# -----------------------------------

#if(WIN32)
#    include_directories("C:/mingw64/x86_64-w64-mingw32/include/")
#endif()

# -----------------------------------
# - compilation spec files	        -
# -----------------------------------

add_subdirectory("lib")
add_subdirectory("resources")
add_subdirectory("src")
