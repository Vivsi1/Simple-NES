cmake_minimum_required(VERSION 3.10)
project(SimpleNES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# ENABLE SAFE COMPILER FLAGS
# -----------------------------

# Debug build: full symbols, no optimization
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -Wall")

# Release build:
# -O2 and -O3 break emulators because of UB. Use O1 (safe).
set(CMAKE_CXX_FLAGS_RELEASE 
    "-O0 -g3 -ggdb -fno-omit-frame-pointer -fno-optimize-sibling-calls \
     -fwrapv -fno-strict-aliasing -fno-strict-overflow")


# Optional: Enable AddressSanitizer (only works on some MinGW builds)
option(USE_ASAN "Enable AddressSanitizer" OFF)
if(USE_ASAN)
    message(STATUS "Building with AddressSanitizer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()



# SDL2 paths
set(SDL2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/SDL2/include)
set(SDL2_LIB_DIR ${CMAKE_SOURCE_DIR}/SDL2/lib)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/mappers/*.cpp"
)

# Remove test files
list(REMOVE_ITEM SOURCES
    "${CMAKE_SOURCE_DIR}/CPUTest.cpp"
    "${CMAKE_SOURCE_DIR}/CPUsst.cpp"
)

add_executable(SimpleNES ${SOURCES})

target_link_libraries(SimpleNES SDL2 SDL2main)
