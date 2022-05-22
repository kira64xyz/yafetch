cmake_minimum_required(VERSION 3.17)
project(yafetch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("-- Detected build type: ${CMAKE_BUILD_TYPE}")

# For my LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(yafetch
    src/yafetch.cpp
)

target_compile_options(yafetch PRIVATE
    -Wall
)

install(TARGETS yafetch DESTINATION bin)
