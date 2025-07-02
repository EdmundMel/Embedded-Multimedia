cmake_minimum_required(VERSION 3.25)
project(home_alarm_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch yaml-cpp
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        yaml-cpp-0.7.0
)

FetchContent_MakeAvailable(yaml-cpp)

# Find PostgreSQL (libpq)
find_package(PostgreSQL REQUIRED)

# Collect all source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(home-alarm-core ${SRC_FILES})

target_include_directories(home-alarm-core
    PRIVATE
        ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(home-alarm-core
    PRIVATE
        yaml-cpp
        ${PostgreSQL_LIBRARIES}
)
