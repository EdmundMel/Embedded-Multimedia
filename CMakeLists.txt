cmake_minimum_required(VERSION 3.31)
project(home_alarm_core VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



include(FetchContent)

# Add yaml-cpp
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

include_directories(include)

add_executable(home-alarm-core
        src/main.cpp
        src/core.cpp
        src/statemachine.cpp
        src/ruleengine.cpp
        src/db_access.cpp
)

target_link_libraries(home-alarm-core PRIVATE yaml-cpp::yaml-cpp)