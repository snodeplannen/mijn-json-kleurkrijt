cmake_minimum_required(VERSION 3.15)
project(wjq VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Simdjson setup
if(TARGET simdjson)
    # Use global simdjson from root project
    message(STATUS "Using global simdjson target")
else()
    # Use local single-header version
    message(STATUS "Using local simdjson (single-header)")
    add_library(simdjson STATIC libs/simdjson/simdjson.cpp)
    target_include_directories(simdjson PUBLIC libs/simdjson)
endif()

# Main executable
add_executable(wjq 
    src/main.cpp
    src/printer.hpp
    src/style.hpp
    src/color.hpp
    src/json_parser.hpp
)

target_include_directories(wjq PRIVATE src)
target_link_libraries(wjq PRIVATE simdjson)

# Platform-specific
if(WIN32)
    target_compile_definitions(wjq PRIVATE _WIN32_WINNT=0x0601)
    # Linked required windows libs if any (prompt suggested shlwapi)
    target_link_libraries(wjq PRIVATE shlwapi)
endif()

# Installation
install(TARGETS wjq DESTINATION bin)
