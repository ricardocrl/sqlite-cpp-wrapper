cmake_minimum_required(VERSION 3.8)
project(SqliteCppWrapper_tests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(GTEST_RUN_FLAGS --gtest_color=yes --gtest_shuffle)

# Download and unpack googletest at configure time
configure_file(test_libs/CMakeLists.txt googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src ${CMAKE_BINARY_DIR}/googletest-build)

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
include_directories("${gtest_SOURCE_DIR}/include"
    "${gmock_SOURCE_DIR}/include")

enable_testing()

# Utility functions
function(configure_test testExecutable)
    # Link against gtest & gmock
    target_link_libraries(${testExecutable} PRIVATE gtest PRIVATE gmock_main)    
    # Create test name as the capitalized form of the test executable
    string(TOUPPER ${testExecutable} testName)

    # Add executable to test suite
    add_test(${testName} ${testExecutable} ${GTEST_RUN_FLAGS})
endfunction(configure_test)

# Common headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_libs/gmock-gtest
)

set(REPOSITORY_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(UNIT_TESTS ${CMAKE_CURRENT_SOURCE_DIR})
set(EXTERNAL_DIR ${REPOSITORY_ROOT}/external)

# SqliteConnection
find_library(sqlite_lib REQUIRED NAMES sqlite3 sqlite)
message(STATUS "SQLite libs: ${sqlite_lib}")

add_executable(sqlite_connection_test
    ${REPOSITORY_ROOT}/src/SqliteConnection.cpp
    ${REPOSITORY_ROOT}/src/SqliteTraits.cpp
    ${REPOSITORY_ROOT}/src/StringUtils.cpp
    ${UNIT_TESTS}/SqliteConnection_test.cpp
)
target_include_directories(sqlite_connection_test PUBLIC 
    ${REPOSITORY_ROOT}/include
)
target_include_directories(sqlite_connection_test SYSTEM PUBLIC 
    ${EXTERNAL_DIR}/sqlite_modern_cpp/hdr
)
target_link_libraries(sqlite_connection_test PRIVATE ${sqlite_lib})
configure_test(sqlite_connection_test)

