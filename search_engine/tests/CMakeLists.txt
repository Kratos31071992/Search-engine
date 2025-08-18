cmake_minimum_required(VERSION 3.20)

project(server_test)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)

set(gtest_disable_pthreads on)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(server_test index_tests.cpp server_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/inverted_index.cpp
        ${CMAKE_SOURCE_DIR}/src/server.cpp)

target_include_directories(server_test PUBLIC ${googletest_SOURCE_DIR}/include ${INCLUDE_DIR})

target_link_libraries(server_test PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(server_test)