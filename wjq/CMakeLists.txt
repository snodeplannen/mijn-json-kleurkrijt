cmake_minimum_required(VERSION 3.15)
project(wjq VERSION 0.8.2 LANGUAGES CXX)

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
    ../src/matchers.cpp
    src/query_engine.cpp
    src/query_parser.cpp
    src/query_executor.cpp
    src/jsonpath_parser.cpp
    src/jsonpath_executor.cpp
)

target_include_directories(wjq PRIVATE src ../src)
target_link_libraries(wjq PRIVATE simdjson)

# Platform-specific
if(WIN32)
    target_compile_definitions(wjq PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(wjq PRIVATE shlwapi)
endif()

# Enable all warnings
if(MSVC)
    target_compile_options(wjq PRIVATE /W4)
else()
    target_compile_options(wjq PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Tests
enable_testing()
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS wjq DESTINATION bin)

# Examples
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    # Just copy example files
    file(GLOB EXAMPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.json")
    if(EXAMPLE_FILES)
        install(FILES ${EXAMPLE_FILES} DESTINATION share/wjq/examples)
    endif()
endif()
