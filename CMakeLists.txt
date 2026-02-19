cmake_minimum_required(VERSION 3.14)

# clang++ over g++ any time
set(CMAKE_CXX_COMPILER_NAMES clang++ g++ icpc c++ cxx)
project(fatpup-cli LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# fatpup controls these options in its own project; force them off here.
set(BUILD_TESTS OFF CACHE BOOL "Build fatpup tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build fatpup examples" FORCE)

FetchContent_Declare(
    fatpup
    GIT_REPOSITORY https://github.com/witaly-iwanow/fatpup.git
    GIT_TAG 844e5e4e791b2b9ef6381f2dda32cbf52a88f491
)

FetchContent_MakeAvailable(fatpup)

add_executable(fatpup-cli
    src/main.cpp
)

target_link_libraries(fatpup-cli PRIVATE fatpup)
