cmake_minimum_required(VERSION 3.12)
project(colored_json)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

# simdjson dependency via FetchContent
include(FetchContent)
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG v3.7.0
)
FetchContent_MakeAvailable(simdjson)

pybind11_add_module(colored_json src/bindings.cpp)

# Link simdjson
target_link_libraries(colored_json PRIVATE simdjson)

# Compile flags voor maximale snelheid
if(MSVC)
    target_compile_options(colored_json PRIVATE
        /O2
        /Ob2
        /GL
        /Gy
        /DNDEBUG
    )
else()
    target_compile_options(colored_json PRIVATE
        -O3
        -march=native
        -ffast-math
        -DNDEBUG
        -Wall
        -Wextra
    )
endif()

