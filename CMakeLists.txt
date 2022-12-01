cmake_minimum_required(VERSION 3.2.0)

project(SqliteCppWrapper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# find SQLite libraries
find_library(sqlite_lib REQUIRED NAMES sqlite3 sqlite)
message(STATUS "SQLite libs: ${sqlite_lib}")

# sqlite-cpp-wrapper library
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*
)

add_library(sqlite-cpp-wrapper ${SOURCE_FILES})

target_include_directories(sqlite-cpp-wrapper PUBLIC ${INCLUDE_DIR})
target_include_directories(sqlite-cpp-wrapper SYSTEM PUBLIC external/sqlite_modern_cpp/hdr)

target_link_libraries(sqlite-cpp-wrapper
    ${sqlite_lib}
)

# unit tests
add_subdirectory(unit-tests/)
