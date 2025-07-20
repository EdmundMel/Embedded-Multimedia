cmake_minimum_required(VERSION 3.31)
project(home_alarm_core VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

FetchContent_Declare(
  cpp_httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.23.1  # lock to a specific version
  SOURCE_SUBDIR  ""       # no subdir; top-level has httplib.h
)
FetchContent_MakeAvailable(cpp_httplib)

# Define interface target
add_library(httplib INTERFACE)
target_include_directories(httplib
  INTERFACE
    ${cpp_httplib_SOURCE_DIR}
)

# PostgreSQL
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

# --- Main executable ---
add_executable(home-alarm-core
    src/*.cpp
)

target_link_libraries(home-alarm-core
    PRIVATE
      PostgreSQL::PostgreSQL
      httplib
)
