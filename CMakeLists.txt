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

# PostgreSQL
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

# --- Main executable ---
add_executable(home-alarm-core
    src/main.cpp
    src/alarm_system.cpp
    src/db_access.cpp
    
)

target_link_libraries(home-alarm-core
    PRIVATE
      PostgreSQL::PostgreSQL
      httplib
)
