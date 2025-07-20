cmake_minimum_required(VERSION 3.31)
project(home_alarm_core VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

FetchContent_Declare(
  cpp_httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.23.1
)
FetchContent_MakeAvailable(cpp_httplib)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

# PostgreSQL & Threads
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

# C++ unit tests
# Library for DB access
add_library(db_access STATIC src/db_access.cpp)

target_link_libraries(db_access PRIVATE
    PostgreSQL::PostgreSQL
    Threads::Threads
)

# Test executable
add_executable(db_access_test src/db_access_test.cpp)
target_link_libraries(db_access_test PRIVATE
    db_access
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(db_access_test)

# Main executable
add_executable(home-alarm-core
    src/main.cpp
    src/alarm_system.cpp
    src/db_access.cpp
)

add_test(NAME DBAccessTest COMMAND db_access_test)

target_link_libraries(home-alarm-core
    PRIVATE
      PostgreSQL::PostgreSQL
      httplib::httplib
)
