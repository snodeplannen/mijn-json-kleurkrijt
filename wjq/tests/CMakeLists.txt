cmake_minimum_required(VERSION 3.15)

# Download Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Test executable
add_executable(wjq_tests
    test_main.cpp
    test_color.cpp
    test_matchers.cpp
    test_style.cpp
    test_style_context.cpp
    test_callbacks.cpp
    test_printer.cpp
    test_streaming.cpp
    test_query.cpp
    ../../src/matchers.cpp
    ../src/query_engine.cpp
    ../src/query_parser.cpp
    ../src/query_executor.cpp
    ../src/jsonpath_parser.cpp
    ../src/jsonpath_executor.cpp
)

target_include_directories(wjq_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(wjq_tests PRIVATE Catch2::Catch2WithMain simdjson)

if(WIN32)
    target_compile_definitions(wjq_tests PRIVATE _WIN32_WINNT=0x0601)
    target_link_libraries(wjq_tests PRIVATE shlwapi)
endif()

# Add tests
include(Catch)
catch_discover_tests(wjq_tests)

# Copy test data
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
